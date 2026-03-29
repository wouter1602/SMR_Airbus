#!/usr/bin/env python
"""
    File loads different positions from a JSON file and gives the user the option to move the robot to each position.
    The robot runs in a separate process to work around the C++ library's GIL-holding behavior.
"""

import multiprocessing as mp
import logging
import asyncio
import signal
import doosan_drfl as drfl
import numpy as np
import sys
from pathlib import Path
import json

from robot_worker import robot_worker, MOVE_TIMEOUT, POLL_INTERVAL
from cognix import CognexCamera

IP_DOOSAN = "192.168.0.50"
PORT = 12345
IP_CAMERA_TCP = "192.168.0.12"

logging.basicConfig(
    level=logging.DEBUG,
    format="%(asctime)s - [%(levelname)s] - %(message)s",
    datefmt="%H:%M:%S",
)
logger = logging.getLogger(__name__)

_waiting_on_mwait: bool = False


# ─────────────────────────────────────────────
#  Async helpers
# ─────────────────────────────────────────────

async def test_function() -> None:
    """Proof-of-concept parallel task — runs while the robot is moving."""
    i = 0
    while i < 20 and not _waiting_on_mwait:
        logger.info(f"[test_function] running... [{i}]")
        await asyncio.sleep(1.0)
        i += 1


async def wait_for_motion_complete(
    result_queue: mp.Queue,
    timeout: float = MOVE_TIMEOUT,
) -> None:
    global _waiting_on_mwait
    loop = asyncio.get_running_loop()
    _waiting_on_mwait = True

    try:
        result = await asyncio.wait_for(
            loop.run_in_executor(None, result_queue.get),
            timeout=timeout,
        )
    finally:
        _waiting_on_mwait = False

    if len(result) == 2:
        logger.debug(f"Stopped based on: {result[0]}. With result: {result[1]}")
    else:
        if result != "done":
            raise RuntimeError(result)

async def execute_poses(pose: dict, command_queue: mp.Queue, result_queue: mp.Queue) -> None:
    move_type = pose["move_type"]
    if move_type not in ("joint", "linear", "force"):
        logger.warning(f"Wrong move type: {move_type}")
        return

    logger.debug(f"Executing pose: {pose['name']} [{move_type}]")
    pose_array = np.array(pose["pose_array"], dtype=np.float32)
    command_queue.put((move_type, pose_array))

    try:
        await asyncio.gather(
            wait_for_motion_complete(result_queue, timeout=MOVE_TIMEOUT),
            test_function(),
        )
    except asyncio.TimeoutError:
        logger.error("Move timed out waiting for motion complete")
        command_queue.put(None)
        raise SystemExit(1)
    except RuntimeError as e:
        logger.error(f"Motion error: {e}")
        command_queue.put(None)
        raise SystemExit(1)

    logger.info("Target Reached!")
    await asyncio.sleep(POLL_INTERVAL)

async def toggle_air(command_queue: mp.Queue, result_queue: mp.Queue) -> None:
    command_queue.put(("toggle_air", drfl.GPIO_CTRLBOX_DIGITAL_INDEX.Index_1))

async def get_array_input(loop) -> np.ndarray | None:
    while True:
        raw = await loop.run_in_executor(None, input, "> ")
        raw = raw.strip()

        if raw == "q":
            return None

        try:
            values = np.fromstring(raw, dtype=np.float32, sep=" ")
            if values.size != 6:
                print(f"Expected 6 elements, got {values.size}. Try again (or 'q' to cancel).")
                continue
            logger.debug(f"Got array: {values}, with shape {values.shape}")
            return values
        except ValueError:
            print("Invalid input. Enter 6 space-separated numbers (or 'q' to cancel).")


async def load_poses(filepath: str | Path) -> list:
    with open(filepath, "r") as f:
        return json.load(f)


async def prompt_pose(poses: list, command_queue: mp.Queue, result_queue: mp.Queue, camera: CognexCamera) -> None:
    loop = asyncio.get_running_loop()
    while True:
        print("\nSelect pose:")
        for i, pose in enumerate(poses, start=1):
            print(f"    [{i}] {pose['name']} [{pose['move_type']}] {pose['pose_array']}")
        print("    [q] quit      [m] toggle air    [t] trigger cam 1    [c] costum coords")

        choice = await loop.run_in_executor(None, input, "> ")
        choice = choice.strip()

        if choice.lower() == "q":
            logger.info("Received quit command")
            break
        elif choice.lower() == "m":
            await toggle_air(command_queue, result_queue)
        elif choice.lower() == "t":
            cells = ["U38", "U39", "U40", "U41", "U42", "U43"]
            results = await camera.trigger_and_read(cells)

            def to_float(v):
                try:
                    return float(v)
                except ValueError:
                    return None

            float_results = [to_float(results[c]) for c in cells]
            values = None if any(v is None for v in float_results) else np.array(float_results, dtype=np.float32)

            logger.info(f"Detected cell @ location: {values}")

            if values is not None:
                tmp_pose = {
                    "move_type": "linear",
                    "pose_array": [values],
                    "name": "Detected cell pose",
                }
                await execute_poses(tmp_pose, command_queue, result_queue)

        elif choice.lower() == "c":
            arg = await get_array_input(loop)
            if arg is not None:
                tmp_pose = {
                    "move_type": "linear",
                    "pose_array": [arg],
                    "name": "Costum pose",
                }
                await execute_poses(tmp_pose, command_queue, result_queue)
            else:
                logger.info("costum coords input cancelled")
        elif choice.isdigit() and 1 <= int(choice) <= len(poses):
            await execute_poses(poses[int(choice) - 1], command_queue, result_queue)
        else:
            logger.info(f"Invalid input. Enter a number between 1 and {len(poses)}, or 'q'.")

# ─────────────────────────────────────────────
#  Main
# ─────────────────────────────────────────────

async def main() -> None:
    loop = asyncio.get_running_loop()
    main_task = asyncio.current_task()

    def _handle_sigint():
        if _waiting_on_mwait:
            logger.info("SIGINT received, waiting for mwait() to finish...")
        main_task.cancel()

    loop.add_signal_handler(signal.SIGINT, _handle_sigint)

    filepath = sys.argv[1] if len(sys.argv) > 1 else "poses.json"
    poses = await load_poses(filepath)

    command_queue: mp.Queue = mp.Queue()
    result_queue: mp.Queue = mp.Queue()

    cam = CognexCamera(IP_CAMERA_TCP,
        username="admin",
        password="",)
    try:
        await cam.connect() # Wait for camera to connect
        logger.info(f"Connected to TCP camera: {IP_CAMERA_TCP}")
    except Exception as e:
        logger.error(f"Failed to connect to camera (TCP): {e}")
        # raise SystemExit(1)

    worker = mp.Process(
        target=robot_worker,
        args=(command_queue, result_queue, IP_DOOSAN, PORT),
        daemon=True,
    )
    worker.start()

    # Wait for worker to finish setup
    result = await loop.run_in_executor(None, result_queue.get)
    if result != "ready":
        logger.error(f"Worker failed during setup: {result}")
        worker.terminate()
        raise SystemExit(1)

    logger.info("---- Ready to Move ----")

    await prompt_pose(poses, command_queue, result_queue, cam)

    command_queue.put(None)  # Graceful shutdown
    worker.join()
    logger.info("Connection closed")


if __name__ == "__main__":
    try:
        asyncio.run(main())
    except asyncio.CancelledError:
        logger.info("Keyboard interrupt, exiting...")
        sys.exit(0)
    except Exception as e:
        logger.error(f"System error: {e}")
        sys.exit(1)
