"""
modbus.py
---------

Modbus TCP communication layer for the Cognex barcode scanner.

Responsibilities:
  - Connect / disconnect / reconnect to the scanner
  - Poll the scan-ready flag register
  - Read barcode characters from holding registers
  - Reset the scan flag after a successful read

"""

# Set-ExecutionPolicy Unrestricted -Scope Process; .venv\Scripts\Activate.ps1
# ---------------------------------------------------
# IMPORTS
# ---------------------------------------------------

import time
import logging
from pymodbus.client import ModbusTcpClient
from pymodbus.exceptions import ModbusException


# ---------------------------------------------------
# CONFIGURATION
# ---------------------------------------------------

SCAN_FLAG_REGISTER    = 32
BARCODE_START_REGISTER = 34
BARCODE_REGISTER_LENGTH = 19
SCANNER_RESET_REGISTER = 3
INSPECTION_REGISTER     = 32
WING_START_REGISTER  = 55
WING_REGISTER_LENGTH = 9

CONNECT_MAX_RETRIES  = 5
CONNECT_RETRY_DELAY  = 3.0   # seconds between retries


# ---------------------------------------------------
# LOGGER
# ---------------------------------------------------

logger = logging.getLogger("barcode_system.modbus")


# ---------------------------------------------------
# SCANNER INTERFACE
# ---------------------------------------------------

class ScannerInterface:
    """
    Handles Modbus TCP communication with the Cognex scanner.

    Supports connect / disconnect / reconnect and exposes
    three scanner operations: scan_available, read_barcode,
    and reset_scan_flag.
    """



    def  __init__(self, ip: str, port: int):
        self.ip   = ip
        self.port = port
        self.client: ModbusTcpClient | None = None

    # ---------------------------------------------------

    def connect(self, max_retries: int = CONNECT_MAX_RETRIES):
        """
        Connect to the scanner with retry logic.

        Raises RuntimeError if all retries are exhausted.
        """
        for attempt in range(1, max_retries + 1):
            logger.info(
                f"Connecting to scanner at {self.ip}:{self.port} "
                f"(attempt {attempt}/{max_retries})..."
            )
            try:
                self.client = ModbusTcpClient(self.ip, port=self.port)
                if self.client.connect():
                    logger.info("Scanner connected successfully.")
                    return
            except Exception as e:
                logger.warning(f"Connection attempt {attempt} failed: {e}")

            if attempt < max_retries:
                logger.info(f"Retrying in {CONNECT_RETRY_DELAY}s...")
                time.sleep(CONNECT_RETRY_DELAY)

        raise RuntimeError(
            f"Could not connect to scanner at {self.ip}:{self.port} "
            f"after {max_retries} attempts."
        )

    # ---------------------------------------------------

    def disconnect(self):
        """Cleanly close the Modbus connection."""
        if self.client:
            self.client.close()
            self.client = None
            logger.info("Scanner disconnected.")

    # ---------------------------------------------------

    def reconnect(self):
        """Disconnect then reconnect."""
        logger.warning("Attempting to reconnect to scanner...")
        self.disconnect()
        self.connect()

    # ---------------------------------------------------

    def _check_connected(self):
        """Raise RuntimeError if the client is not initialised."""
        if self.client is None:
            raise RuntimeError("Scanner is not connected.")

    # ---------------------------------------------------

    def scan_available(self) -> bool:
        """Return True if the scan-flag register is set to 1."""
        self._check_connected()

        result = self.client.read_holding_registers(SCAN_FLAG_REGISTER, count=1)

        if result.isError():
            raise ModbusException("Error reading scan flag register.")

        return result.registers[0] != 0

    # ---------------------------------------------------

    def read_barcode(self):
        """
        Read barcode ASCII characters from holding registers.

        Each register holds one character (as an integer code point).
        Reading stops at the first null (0) register.

        Returns the stripped barcode string, or None if empty.
        """
        self._check_connected()
        """
        Read barcode ASCII characters from holding registers.

        Each register holds one character (as an integer code point).
        Reading stops at the first null (0) register.

        Returns the stripped barcode string, or None if empty.
        """
        self._check_connected()

        result = self.client.read_holding_registers(
            BARCODE_START_REGISTER,
            count=BARCODE_REGISTER_LENGTH
        )

        if result.isError():
            raise ModbusException("Error reading barcode registers.")

        barcode = ""
        for reg in result.registers:
            if reg == 0:
                break
            barcode += chr(reg)

        return barcode.strip().strip('"') or None

    # ---------------------------------------------------

    def read_inspection(self) -> str:
        """Read inspection result from register 4. 1 = PASS, 0 = FAIL."""
        self._check_connected()
        result = self.client.read_holding_registers(INSPECTION_REGISTER, count=1)
        if result.isError():
            raise ModbusException("Error reading inspection register.")
        return "PASS" if result.registers[0] == 1 else "FAIL"
    def read_wing_id(self) -> str | None:
        """Read wing ID from registers 21-29."""
        self._check_connected()
        result = self.client.read_holding_registers(WING_START_REGISTER, count=WING_REGISTER_LENGTH)
        if result.isError():
            raise ModbusException("Error reading wing ID registers.")
        wing = ""
        for reg in result.registers:
            if reg == 0:
                break
            wing += chr(reg)
        return wing.strip().strip('"') or None
    def reset_scan_flag(self):
        """Clear the scanner ready flag after a successful read."""
        self._check_connected()
        self.client.write_register(SCANNER_RESET_REGISTER, 0)