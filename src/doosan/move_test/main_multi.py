#!/usr/bin/env python
"""
    File loads different positions from a JSON file and gives the user the option to move the robot to each position.
    The robot runs in a separate process to work around the C++ library's GIL-holding behavior.
"""

import multiprocessing as mp
import logging
import asyncio
import signal
import numpy as np
import sys
from pathlib import Path
import json

from robot_worker import robot_worker, MOVE_TIMEOUT, POLL_INTERVAL

IP_ADDRESS = "192.168.8.50"
PORT = 12345

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

    if result != "done":
        raise RuntimeError(result)


async def execute_poses(pose: dict, command_queue: mp.Queue, result_queue: mp.Queue) -> None:
    move_type = pose["move_type"]
    if move_type not in ("joint", "linear"):
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


async def load_poses(filepath: str | Path) -> list:
    with open(filepath, "r") as f:
        return json.load(f)


async def prompt_pose(poses: list, command_queue: mp.Queue, result_queue: mp.Queue) -> None:
    loop = asyncio.get_running_loop()
    while True:
        print("\nSelect pose:")
        for i, pose in enumerate(poses, start=1):
            print(f"    [{i}] {pose['name']} [{pose['move_type']}] {pose['pose_array']}")
        print("    [q] quit")

        choice = await loop.run_in_executor(None, input, "> ")
        choice = choice.strip()

        if choice.lower() == "q":
            logger.info("Received quit command")
            break
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

    worker = mp.Process(
        target=robot_worker,
        args=(command_queue, result_queue, IP_ADDRESS, PORT),
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

    await prompt_pose(poses, command_queue, result_queue)

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
