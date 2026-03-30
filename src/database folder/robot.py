"""
robot.py
--------

Inter-process signal interface for the inspection system.

Instead of Modbus TCP, this module communicates with the main robot
script running on the same machine via three multiprocessing.Event
objects — one per inspection stage:

  evt_wing_ready   — robot has positioned the wing under the QR camera
  evt_cell_ready   — robot has moved the next cell under the scan camera
  evt_cell_placed  — robot has placed the cell; placement camera may fire

Handshake protocol
------------------
  1. Robot script calls event.set() when it is ready.
  2. RobotInterface.wait_for_signal() blocks until the event is set.
  3. RobotInterface.acknowledge_signal() clears the event, telling
     the robot script it may continue.

Wiring (in whichever script owns the processes)
-----------------------------------------------
  from multiprocessing import Event, Process

  evt_wing_ready  = Event()
  evt_cell_ready  = Event()
  evt_cell_placed = Event()

  # Pass events to the inspection system
  from robot import RobotInterface
  robot = RobotInterface(evt_wing_ready, evt_cell_ready, evt_cell_placed)

  # Pass the same events to the robot process so it can set() them
  robot_process = Process(
      target=robot_main,
      args=(evt_wing_ready, evt_cell_ready, evt_cell_placed)
  )
"""

# ---------------------------------------------------
# IMPORTS
# ---------------------------------------------------

import logging
from multiprocessing.synchronize import Event as EventType


# ---------------------------------------------------
# CONFIGURATION
# ---------------------------------------------------

# How long (seconds) to wait for a signal before raising TimeoutError.
# Set to None to wait forever.
SIGNAL_TIMEOUT = 60.0


# ---------------------------------------------------
# LOGGER
# ---------------------------------------------------

logger = logging.getLogger("barcode_system.robot")


# ---------------------------------------------------
# REGISTER CONSTANTS
# (kept as named indices so main.py imports don't break)
# ---------------------------------------------------

REG_WING_READY  = 0
REG_CELL_READY  = 1
REG_CELL_PLACED = 2


# ---------------------------------------------------
# ROBOT INTERFACE
# ---------------------------------------------------

class RobotInterface:
    """
    Wraps three multiprocessing.Event objects as a signal interface.

    The events are created externally (by the process that also owns
    the robot script) and passed in here. This class never creates
    or destroys them — it only waits on and clears them.

    Parameters
    ----------
    evt_wing_ready  : Event set by robot when wing is under Camera 1
    evt_cell_ready  : Event set by robot when a cell is under Camera 1
    evt_cell_placed : Event set by robot when a cell has been placed
    timeout         : Seconds to wait before raising TimeoutError (None = forever)
    """

    def __init__(
        self,
        evt_wing_ready:  EventType,
        evt_cell_ready:  EventType,
        evt_cell_placed: EventType,
        timeout: float | None = SIGNAL_TIMEOUT,
    ):
        self._events: dict[int, EventType] = {
            REG_WING_READY:  evt_wing_ready,
            REG_CELL_READY:  evt_cell_ready,
            REG_CELL_PLACED: evt_cell_placed,
        }
        self._timeout = timeout

    # ── lifecycle stubs ───────────────────────────────────────────
    # Kept so main.py can call connect() / disconnect() / reconnect()
    # without any changes.

    def connect(self, **kwargs):
        logger.info("RobotInterface ready (multiprocessing.Event mode).")

    def disconnect(self):
        pass

    def reconnect(self):
        logger.warning("RobotInterface reconnect called (no-op in Event mode).")

    # ── signal helpers ────────────────────────────────────────────

    def wait_for_signal(self, register: int, timeout: float | None = None):
        """
        Block until the event for this register is set.

        Parameters
        ----------
        register : one of REG_WING_READY, REG_CELL_READY, REG_CELL_PLACED
        timeout  : override the instance-level timeout for this call

        Raises
        ------
        TimeoutError  if the event is not set within the timeout
        KeyError      if register is not one of the three known values
        """
        event     = self._get_event(register)
        wait_secs = timeout if timeout is not None else self._timeout

        label = self._label(register)
        logger.debug(f"Waiting for signal: {label} …")

        signalled = event.wait(timeout=wait_secs)

        if not signalled:
            raise TimeoutError(
                f"Timed out waiting for robot signal '{label}' "
                f"(waited {wait_secs:.0f} s)."
            )

        logger.debug(f"Signal received: {label}")

    def acknowledge_signal(self, register: int):
        """
        Clear the event to signal back to the robot script that
        this inspection stage is complete and it may continue.
        """
        event = self._get_event(register)
        event.clear()
        logger.debug(f"Signal acknowledged (cleared): {self._label(register)}")

    # ── internal ──────────────────────────────────────────────────

    def _get_event(self, register: int) -> EventType:
        if register not in self._events:
            raise KeyError(
                f"Unknown register index {register}. "
                f"Expected one of {list(self._events.keys())}."
            )
        return self._events[register]

    @staticmethod
    def _label(register: int) -> str:
        return {
            REG_WING_READY:  "WING_READY",
            REG_CELL_READY:  "CELL_READY",
            REG_CELL_PLACED: "CELL_PLACED",
        }.get(register, f"REGISTER_{register}")
