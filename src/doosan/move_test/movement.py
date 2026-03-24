#!/usr/bin/env python3

import multiprocessing as mp
import logging
import asyncio
import signal
import numpy as np
import sys
from pathlib import Path
import json

import doosan_drfl as drfl
from robot_worker import robot_worker, MOVE_TIMEOUT, POLL_INTERVAL
from cognix import CognexCamera

# Setup varables
IP_DOOSAN = "192.168.0.50"
PORT_DOOSAN = 12345
IP_CAMERA_TCP = "192.168.0.12"

logging.basicConfig(
    level=logging.DEBUG,
    format="%(asctime)s - [%(levelname)s] - %(message)s",
    datefmt="%H:%M:%S",
)
logger = logging.getLogger(__name__)

_currently_moving: bool = False


async def wait_for_motion_complete(
    result_queue: mp.Queue,
    timeout: float = MOVE_TIMEOUT,
) -> None:
    global _currently_moving
    loop = asyncio.get_running_loop()
    _currently_moving = True

    try:
        result = await asyncio.wait_for(
            loop.run_in_executor(None, result_queue.get),
            timeout=timeout,
        )
    finally:
        _currently_moving = False

    if result != "done":
        raise RuntimeError(result)

async def execute_poses(
    pose: dict,
    command_queue: mp.Queue,
    result_queue: mp.Queue
) -> None:
    move_type = pose["move_type"]
    if move_type not in ("joint", "linear"):
        logger.warning(f"Wrong move type: {move_type}")
        return

    logger.debug(f"Executing pose: {pose['name']} [{move_type}]")
    pose_array = np.array(pose['pose_array'], dtype=np.float32)
    command_queue.put((move_type, pose_array))

    try:
        await wait_for_motion_complete(result_queue=result_queue, timeout=MOVE_TIMEOUT)
    except asyncio.TimeoutError:
        logger.error("Move timed out waiting for motion complete")
        command_queue.put(None) # Shutsdown robot worker thread
        raise SystemExit(1)
    except RuntimeError as e:
        logger.error(f"Motion error: {e}")
        command_queue.put(None) #Shutsdowns robot worker thread
        raise SystemExit(1)

    logger.info("Target Reached!")
    await asyncio.sleep(POLL_INTERVAL)

async def toggle_air(
    command_queue: mp.Queue,
    result_queue: mp.Queue,
    index: drfl.GPIO_CTRLBOX_DIGITAL_INDEX = drfl.GPIO_CTRLBOX_DIGITAL_INDEX.Index_1
) -> None:
    command_queue.put(("toggle_air", index))

async def load_poses(filepath: str | Path
) -> list:
    with open(filepath, "r") as f:
        return json.load(f)


async def main() -> None:
    loop = asyncio.get_event_loop()
    main_task = asyncio.current_task()

    def _handle_sigint():
        if _currently_moving: #System is currently moving
            logger.info("SIGIN received, waiting for movment to finish...") #TODO: Send emidiat stop command.
        main_task.cancel()

    loop.add_signal_handler(signal.SIGINT, _handle_sigint)

    filepath = sys.argv[1] if len(sys.argv) > 1 else "poses.json"
    poses = await load_poses(filepath)

    command_queue: mp.Queue = mp.Queue()
    result_queue: mp.Queue = mp.Queue()

    camera_1 = CognexCamera(ip = IP_CAMERA_TCP,
        username="admin",
        password="",
    )

    try:
        await camera_1.connect() # Wait for camera to connect
    except Exception as e:
        logger.error(f"Failed to connect to camera (TCP): {e}")
        raise SystemExit(1)

    logger.info(f"Connect to TCP camera: {IP_CAMERA_TCP}.")

    worker = mp.Process(
        target=robot_worker,
        args=(command_queue, result_queue, IP_DOOSAN, PORT_DOOSAN),
        daemon=True,
    )

    worker.start()

    # Wait for worker to finish setup
    result = await loop.run_in_executor(None, result_queue.get)
    if result != "Read":
        logger.error(f"Worker failed during setup: {result}")
        worker.terminate()
        raise SystemExit(1)

    logger.info("--- Ready to Move ---")

    # Do something

    command_queue.put(None) # Gracefull shutdown of worker
    worker.join() # Wait for shutdown
    logger.info("Connection closed") # TODO: close Cognix camera


if __name__ == "__main__":
    try:
        asyncio.run(main())
    except asyncio.CancelledError:
        logger.info("Keyboard interupt, exiting...")
        sys.exit(0)
    except Exception as e:
        logger.error(f"System error: {e}")
        sys.exit(1)
