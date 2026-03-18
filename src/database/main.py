"""
main.py
-------

Industrial Barcode Logging System — main controller.

Reads barcode data from a Cognex scanner via Modbus TCP
and logs each scan into an Excel workbook.

System responsibilities:
  1. Wait for a barcode scan
  2. Read the barcode from Modbus registers
  3. Determine the placement slot (1-16)
  4. Perform inspection logic
  5. Store the barcode + inspection result in Excel

Robot placement is handled by a separate system.

Modules
-------
  modbus.py   — Modbus TCP scanner communication
  database.py — Excel read/write operations
  main.py     — Orchestration, inspection, duplicate detection, slot tracking

Author: Example Industrial System
"""

# ---------------------------------------------------
# IMPORTS
# ---------------------------------------------------

import time
import logging
from pymodbus.exceptions import ModbusException

from modbus import ScannerInterface
from database import ExcelManager          # ← FIX: was missing


# ---------------------------------------------------
# SYSTEM CONFIGURATION
# ---------------------------------------------------

SCANNER_IP   = "192.168.0.10"  # replace with actual scanner IP address
MODBUS_PORT  = 502

EXCEL_FILE   = "wing_barcode_database.xlsx"

SCAN_POLL_INTERVAL   = 0.2   # seconds between polls when idle
MAX_CONSECUTIVE_ERRORS = 10  # reconnect after this many Modbus errors in a row

MAX_BARCODES_PER_WING = 16


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
    """
    Prevents duplicate barcode entries within a session.

    Note: the seen-set is in-memory only and clears on process restart.
    If persistence across restarts is required, back this with a file or DB.
    """

    def __init__(self):
        self._seen: set[str] = set()

    def is_duplicate(self, barcode: str) -> bool:
        """Return True if barcode was already seen; register it if not."""
        if barcode in self._seen:
            return True
        self._seen.add(barcode)
        return False

    def reset(self):
        """Clear all seen barcodes (e.g. at the start of a new batch)."""
        self._seen.clear()
        logger.info("Duplicate detector reset.")


# ---------------------------------------------------
# SLOT MANAGER
# ---------------------------------------------------

class SlotManager:
    """
    Tracks the current slot (1-16) and wing number (1-based).

    Calling next_slot() advances to the next slot.
    After slot 16 the counter wraps to slot 1 and the wing increments.
    """

    def __init__(self):
        self.current_slot = 1
        self.current_wing = 1

    def next_slot(self):
        """Advance to the next slot; start a new wing when the current one is full."""
        self.current_slot += 1
        if self.current_slot > MAX_BARCODES_PER_WING:
            self.current_slot = 1
            self.current_wing += 1
            logger.info(
                f"Wing {self.current_wing - 1} complete. "
                f"Starting wing {self.current_wing}."
            )

    @property
    def slot(self) -> int:
        return self.current_slot

    @property
    def wing(self) -> int:
        return self.current_wing


# ---------------------------------------------------
# MAIN CONTROLLER
# ---------------------------------------------------

class BarcodeSystem:
    """
    Main system controller.

    Orchestrates scanning (modbus.py), persistence (database.py),
    inspection, duplicate detection, and slot management.
    Reconnects automatically after repeated Modbus failures.
    """

    def __init__(self):
        self.scanner    = ScannerInterface(SCANNER_IP, MODBUS_PORT)
        self.excel      = ExcelManager(EXCEL_FILE)   # ← FIX: was missing
        self.duplicates = DuplicateDetector()
        self.slots      = SlotManager()
        self._consecutive_errors = 0

    # ---------------------------------------------------

    def start(self):
        """Connect to the scanner and enter the main polling loop."""
        logger.info("Starting barcode logging system.")
        self.scanner.connect()

        while True:
            try:
                self._poll_once()
                self._consecutive_errors = 0

            except ModbusException as e:
                self._consecutive_errors += 1
                logger.error(
                    f"Modbus error ({self._consecutive_errors}/{MAX_CONSECUTIVE_ERRORS}): {e}"
                )
                if self._consecutive_errors >= MAX_CONSECUTIVE_ERRORS:
                    logger.warning("Too many consecutive Modbus errors — reconnecting.")
                    self._safe_reconnect()
                    self._consecutive_errors = 0
                else:
                    time.sleep(2)

            except Exception as e:
                logger.exception(f"Unexpected error: {e}")
                time.sleep(2)

    # ---------------------------------------------------
    def _poll_once(self):
        barcode = self.scanner.read_barcode()
        
        if not barcode:
            time.sleep(SCAN_POLL_INTERVAL)
            return

        if self.duplicates.is_duplicate(barcode):
            time.sleep(SCAN_POLL_INTERVAL)
            return

        wing_id    = self.scanner.read_wing_id()
        inspection = self.scanner.read_inspection()
        
        logger.info(f"Wing: {wing_id} | Barcode: {barcode} | Inspection: {inspection}")

        self.excel.write_barcode(
            wing_id    = wing_id,
            slot       = self.slots.slot,
            barcode    = barcode,
            inspection = inspection,
        )

        self.slots.next_slot()

    # ---------------------------------------------------

    def _safe_reconnect(self):
        """Try to reconnect without raising — logs failure and continues."""
        try:
            self.scanner.reconnect()
        except RuntimeError as e:
            logger.error(f"Reconnect failed: {e}. Will retry on next error cycle.")


# ---------------------------------------------------
# PROGRAM ENTRY
# ---------------------------------------------------

def main():
    system = BarcodeSystem()
    system.start()


if __name__ == "__main__":
    main()
