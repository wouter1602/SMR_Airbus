"""
main.py
-------

Industrial Solar-Cell Inspection System — main controller.

Process flow (per wing)
-----------------------

  STEP 1 — Wing QR scan
    * Wait for robot signal: wing is in QR-scan position  (evt_wing_ready)
    * Trigger Camera 1  ->  read wing QR code
    * Acknowledge signal so robot may continue

  STEP 2+3 — Cell loop  (repeated 16x per wing)

    STEP 2 — Cell barcode scan
      * Wait for robot signal: cell is under the camera   (evt_cell_ready)
      * Trigger Camera 1  ->  read solar-cell barcode
      * Acknowledge signal so robot may place the cell

    STEP 3 — Placement check
      * Wait for robot signal: cell has been placed       (evt_cell_placed)
      * Trigger Camera 2  ->  read PASS / FAIL
      * Acknowledge signal so robot may move to next cell
      * Write wing QR + barcode + result to Excel

Modules
-------
  cognex.py   -- Cognex In-Sight ISNM driver (shared by both cameras)
  robot.py    -- multiprocessing.Event signal interface
  database.py -- Excel persistence
  main.py     -- Orchestration
"""

# ---------------------------------------------------
# IMPORTS
# ---------------------------------------------------

import time
import logging
from multiprocessing import Event

from cognex   import CognexCamera
from robot    import RobotInterface, REG_WING_READY, REG_CELL_READY, REG_CELL_PLACED
from database import ExcelManager


# ---------------------------------------------------
# HARDWARE CONFIGURATION
# ---------------------------------------------------

# Camera 1: reads the wing QR code and the 16 cell barcodes
CAMERA_SCAN_IP       = "192.168.0.10"
CAMERA_SCAN_USER     = "admin"
CAMERA_SCAN_PASSWORD = ""

# Camera 2: checks placement correctness after each cell is placed
CAMERA_PLACE_IP       = "192.168.0.12"
CAMERA_PLACE_USER     = "admin"
CAMERA_PLACE_PASSWORD = ""

# In-Sight spreadsheet cell addresses -- adjust to match your job layout
CELL_WING_QR    = "B0"   # wing QR code          (Camera 1)
CELL_BARCODE    = "C0"   # solar-cell barcode     (Camera 1)
CELL_INSPECTION = "D0"   # placement result       (Camera 2)  "1" = PASS

# Seconds to wait after trigger before reading cells
TRIGGER_SETTLE_S = 1.0


# ---------------------------------------------------
# SYSTEM CONFIGURATION
# ---------------------------------------------------

EXCEL_FILE             = "wing_barcode_database.xlsx"
MAX_BARCODES_PER_WING  = 16
MAX_CONSECUTIVE_ERRORS = 10
RECONNECT_RETRY_DELAY  = 5.0


# ---------------------------------------------------
# LOGGING SETUP
# ---------------------------------------------------

logging.basicConfig(
    level=logging.INFO,
    format="%(asctime)s [%(levelname)s] %(message)s"
)

logger = logging.getLogger("barcode_system")


# ---------------------------------------------------
# DUPLICATE DETECTOR
# ---------------------------------------------------

class DuplicateDetector:
    """Prevents the same barcode being logged twice in a session."""

    def __init__(self):
        self._seen: set[str] = set()

    def is_duplicate(self, barcode: str) -> bool:
        if barcode in self._seen:
            return True
        self._seen.add(barcode)
        return False

    def reset(self):
        self._seen.clear()
        logger.info("Duplicate detector reset.")


# ---------------------------------------------------
# MAIN SYSTEM
# ---------------------------------------------------

class InspectionSystem:
    """
    Orchestrates the full 3-step inspection cycle.

    Parameters
    ----------
    evt_wing_ready  : multiprocessing.Event -- set by robot when wing is in position
    evt_cell_ready  : multiprocessing.Event -- set by robot when cell is under camera
    evt_cell_placed : multiprocessing.Event -- set by robot when cell has been placed
    """

    def __init__(self, evt_wing_ready, evt_cell_ready, evt_cell_placed):
        self._cam_scan = CognexCamera(
            CAMERA_SCAN_IP,
            username=CAMERA_SCAN_USER,
            password=CAMERA_SCAN_PASSWORD,
        )
        self._cam_place = CognexCamera(
            CAMERA_PLACE_IP,
            username=CAMERA_PLACE_USER,
            password=CAMERA_PLACE_PASSWORD,
        )
        self._robot = RobotInterface(
            evt_wing_ready  = evt_wing_ready,
            evt_cell_ready  = evt_cell_ready,
            evt_cell_placed = evt_cell_placed,
        )
        self._excel      = ExcelManager(EXCEL_FILE)
        self._duplicates = DuplicateDetector()
        self._wing_count = 0
        self._consecutive_errors = 0

    # ---- startup ------------------------------------------------

    def start(self):
        logger.info("=== Inspection system starting ===")
        self._cam_scan.connect()
        self._cam_place.connect()
        self._robot.connect()
        logger.info("All hardware connected. Waiting for first wing ...")

        while True:
            try:
                self._process_wing()
                self._consecutive_errors = 0

            except TimeoutError as e:
                # Signal never arrived -- stay on the same wing counter,
                # just log and retry. Do NOT increment _wing_count here.
                logger.error(f"Robot signal timeout: {e} -- waiting for signal again.")
                time.sleep(RECONNECT_RETRY_DELAY)

            except Exception as e:
                self._consecutive_errors += 1
                logger.exception(
                    f"Unexpected error ({self._consecutive_errors}/"
                    f"{MAX_CONSECUTIVE_ERRORS}): {e}"
                )
                if self._consecutive_errors >= MAX_CONSECUTIVE_ERRORS:
                    logger.warning("Too many errors -- reconnecting cameras.")
                    self._safe_reconnect_cameras()
                    self._consecutive_errors = 0
                else:
                    time.sleep(RECONNECT_RETRY_DELAY)

    # ---- wing-level logic ---------------------------------------

    def _process_wing(self):
        """
        Run the full inspection cycle for one wing.

        NOTE: _wing_count is incremented AFTER the WING_READY signal
        arrives so that a timeout does not create a phantom wing.
        """
        logger.info("--- Waiting for wing QR scan position ---")

        # STEP 1: block until robot signals wing is in position
        self._robot.wait_for_signal(REG_WING_READY)

        # Only count the wing once we know it is actually there
        self._wing_count += 1
        logger.info(
            f"--- Wing {self._wing_count}: signal received, triggering Camera 1 ---"
        )

        self._cam_scan.trigger()
        time.sleep(TRIGGER_SETTLE_S)
        wing_qr = self._cam_scan.read_cell(CELL_WING_QR)

        if wing_qr:
            logger.info(f"Wing QR code: {wing_qr}")
        else:
            wing_qr = f"Wing_{self._wing_count}"
            logger.warning(f"Wing QR empty -- using fallback ID '{wing_qr}'.")

        self._robot.acknowledge_signal(REG_WING_READY)

        # STEP 2+3: 16 cells
        for slot in range(1, MAX_BARCODES_PER_WING + 1):
            self._process_cell(wing_qr, slot)

        logger.info(f"--- Wing {self._wing_count} ({wing_qr}) complete ---")

    # ---- cell-level logic ---------------------------------------

    def _process_cell(self, wing_qr: str, slot: int):
        logger.info(
            f"  Cell {slot}/{MAX_BARCODES_PER_WING}: "
            f"waiting for barcode scan position ..."
        )

        # STEP 2: Cell barcode
        self._robot.wait_for_signal(REG_CELL_READY)
        logger.info(f"  Cell {slot}: triggering Camera 1 ...")

        self._cam_scan.trigger()
        time.sleep(TRIGGER_SETTLE_S)
        barcode = self._cam_scan.read_cell(CELL_BARCODE)

        if not barcode:
            barcode = f"UNKNOWN_W{self._wing_count}_S{slot}"
            logger.warning(f"  Cell {slot}: barcode empty -- recorded as {barcode}.")
        elif self._duplicates.is_duplicate(barcode):
            logger.warning(f"  Cell {slot}: duplicate barcode detected ({barcode}).")

        self._robot.acknowledge_signal(REG_CELL_READY)
        logger.info(f"  Cell {slot}: barcode = {barcode}")

        # STEP 3: Placement check
        logger.info(f"  Cell {slot}: waiting for placement signal ...")
        self._robot.wait_for_signal(REG_CELL_PLACED)
        logger.info(f"  Cell {slot}: triggering Camera 2 ...")

        self._cam_place.trigger()
        time.sleep(TRIGGER_SETTLE_S)
        raw_result = self._cam_place.read_cell(CELL_INSPECTION)
        inspection = "PASS" if raw_result and raw_result.strip() == "1" else "FAIL"

        self._robot.acknowledge_signal(REG_CELL_PLACED)
        logger.info(f"  Cell {slot}: placement = {inspection}  (raw={raw_result!r})")

        # Write to Excel
        self._excel.write_barcode(
            wing_id    = wing_qr,
            slot       = slot,
            barcode    = barcode,
            inspection = inspection,
        )

    # ---- reconnect helpers --------------------------------------

    def _safe_reconnect_cameras(self):
        for name, fn in [
            ("Camera 1 (scan)",  self._cam_scan.connect),
            ("Camera 2 (place)", self._cam_place.connect),
        ]:
            try:
                fn()
                logger.info(f"Reconnected: {name}")
            except Exception as e:
                logger.error(f"Reconnect failed for {name}: {e}")


# ---------------------------------------------------
# PROGRAM ENTRY
# ---------------------------------------------------

def main():
    # Create the three shared events.
    # The robot script on this machine must receive these same Event
    # objects and call .set() on them at the right moments.
    evt_wing_ready  = Event()
    evt_cell_ready  = Event()
    evt_cell_placed = Event()

    # ----------------------------------------------------------------
    # Wire up your robot process here, passing the events to it, e.g.:
    #
    #   from robot_script import robot_main
    #   from multiprocessing import Process
    #   robot_proc = Process(
    #       target = robot_main,
    #       args   = (evt_wing_ready, evt_cell_ready, evt_cell_placed),
    #   )
    #   robot_proc.start()
    # ----------------------------------------------------------------

    system = InspectionSystem(evt_wing_ready, evt_cell_ready, evt_cell_placed)
    system.start()


if __name__ == "__main__":
    main()
