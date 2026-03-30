"""
cognex.py
---------

Cognex In-Sight camera driver — ISNM protocol (TCP port 23).

Responsibilities:
  - Connect / disconnect to the camera over Telnet-style TCP
  - Authenticate with username + password
  - Trigger the camera (SW8 command)
  - Read spreadsheet cell values (GV command)
  - Fetch a live image over HTTP (requires Pillow)

Cell mapping (adjust to match your In-Sight job layout):
  CELL_BARCODE    = "B0"   →  solar-cell barcode
  CELL_WING_QR    = "C0"   →  wing QR code
  CELL_INSPECTION = "D0"   →  placement check  ("1" = PASS, anything else = FAIL)
"""

# ---------------------------------------------------
# IMPORTS
# ---------------------------------------------------

import io
import socket
import time
import urllib.request
import logging

try:
    from PIL import Image
except ImportError:
    Image = None


# ---------------------------------------------------
# CELL MAPPING  ← adjust these to match your job
# ---------------------------------------------------

CELL_BARCODE    = "B0"   # solar-cell barcode
CELL_WING_QR    = "C0"   # wing QR code
CELL_INSPECTION = "D0"   # placement pass/fail  ("1" = PASS)


# ---------------------------------------------------
# LOGGER
# ---------------------------------------------------

logger = logging.getLogger("barcode_system.cognex")


# ---------------------------------------------------
# COGNEX CAMERA
# ---------------------------------------------------

class CognexCamera:
    """
    Low-level Cognex In-Sight driver using the ISNM protocol (TCP/23).

    Usage
    -----
        cam = CognexCamera("192.168.0.12", username="admin", password="")
        cam.connect()

        cam.trigger()
        barcode    = cam.read_cell(CELL_BARCODE)
        wing_qr    = cam.read_cell(CELL_WING_QR)
        inspection = cam.read_cell(CELL_INSPECTION)   # "1" or "0"

        cam.disconnect()
    """

    def __init__(self, ip: str, username: str = "admin", password: str = ""):
        self.ip       = ip
        self.username = username
        self.password = password
        self._sock: socket.socket | None = None

    # ── connection ────────────────────────────────────────────────

    def connect(self):
        """Open a TCP connection to port 23 and authenticate."""
        logger.info(f"Connecting to Cognex camera at {self.ip}:23 …")
        self._sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self._sock.settimeout(5)
        self._sock.connect((self.ip, 23))

        self._read_until(b"User: ")
        self._sock.sendall((self.username + "\r\n").encode())

        after_user = self._read_until(b"Password: ", b"User Logged In", b"Invalid")
        if b"User Logged In" in after_user:
            logger.info("Cognex camera connected (no password required).")
            self._go_online()
            return

        self._sock.sendall((self.password + "\r\n").encode())
        after_pass = self._read_until(b"User Logged In", b"Invalid Password")

        if b"User Logged In" not in after_pass:
            raise ConnectionError("Cognex login failed — check username/password.")

        logger.info("Cognex camera connected and authenticated.")
        self._go_online()

    def disconnect(self):
        """Close the TCP connection."""
        if self._sock:
            self._sock.close()
            self._sock = None
            logger.info("Cognex camera disconnected.")

    def _go_online(self):
        """Cycle offline → online so the camera is ready to acquire."""
        self._send("SO0")
        time.sleep(0.3)
        self._send("SO1")
        time.sleep(0.8)

    # ── camera actions ────────────────────────────────────────────

    def trigger(self) -> bool:
        """
        Fire a software trigger (SW8).
        Returns True if the camera accepted the trigger.
        """
        resp = self._send("SW8")
        ok   = resp.strip() == "1"
        if not ok:
            logger.warning(f"Trigger response unexpected: {resp!r}")
        return ok

    def read_cell(self, cell: str) -> str | None:
        """
        Read a single In-Sight spreadsheet cell by address.

        Examples: read_cell("B0"), read_cell("C0"), read_cell("D5")

        Returns the cell value as a string, or None on error/empty.
        """
        cell = cell.strip().upper()
        col  = "".join(c for c in cell if c.isalpha())
        row  = "".join(c for c in cell if c.isdigit()) or "0"
        cmd  = f"GV{col}{int(row):03d}"

        self._sock.sendall((cmd + "\r\n").encode())
        raw   = self._read_lines(2)
        lines = raw.decode(errors="replace").strip().splitlines()

        if not lines or lines[0].strip() != "1":
            logger.debug(f"Cell {cell} not found or error — raw: {raw!r}")
            return None

        return lines[1].strip() if len(lines) >= 2 else None

    def trigger_and_read(self, cells: list[str], settle_s: float = 1.0) -> dict:
        """
        Fire the camera and read the requested cells after a short settle delay.

        Returns a dict keyed by cell name, e.g.:
            {"B0": "ABC123", "C0": "QR456", "D0": "1"}
        """
        self.trigger()
        time.sleep(settle_s)
        return {cell: self.read_cell(cell) for cell in cells}

    def set_online(self):
        self._send("SO1")
        time.sleep(0.5)

    def set_offline(self):
        self._send("SO0")

    # ── image helpers ─────────────────────────────────────────────

    def get_image(self):
        """
        Fetch the current camera image over HTTP.
        Returns a PIL Image object (requires Pillow).
        """
        if Image is None:
            raise ImportError("Install Pillow:  pip install Pillow")

        opener = urllib.request.build_opener()
        if self.username:
            pm = urllib.request.HTTPPasswordMgrWithDefaultRealm()
            pm.add_password(None, f"http://{self.ip}/", self.username, self.password)
            opener.add_handler(urllib.request.HTTPBasicAuthHandler(pm))

        for path in ["/image.bmp", "/snapshot", "/GetImage.cgi", "/image.jpg"]:
            try:
                with opener.open(f"http://{self.ip}{path}", timeout=4) as r:
                    return Image.open(io.BytesIO(r.read()))
            except Exception:
                continue

        raise ConnectionError(f"Could not fetch image from {self.ip}")

    def show_image(self):
        """Fetch and display the live image (requires Pillow)."""
        self.get_image().show()

    # ── internal helpers ──────────────────────────────────────────

    def _send(self, command: str) -> str:
        """Send a command and read back the single-line response."""
        self._sock.sendall((command + "\r\n").encode())
        self._sock.settimeout(5)
        buf = b""
        try:
            while True:
                chunk = self._sock.recv(4096)
                if not chunk:
                    break
                buf += chunk
                if buf.endswith(b"\r\n"):
                    break
        except socket.timeout:
            pass
        return buf.decode(errors="replace").strip()

    def _read_until(self, *markers: bytes, timeout: float = 4.0) -> bytes:
        """Read until one of the byte-string markers appears in the buffer."""
        self._sock.settimeout(0.5)
        buf      = b""
        deadline = time.time() + timeout
        while time.time() < deadline:
            try:
                buf += self._sock.recv(4096)
                if any(m in buf for m in markers):
                    break
            except socket.timeout:
                continue
        return buf

    def _read_lines(self, n: int, timeout: float = 5.0) -> bytes:
        """Read until at least n newline-terminated lines are in the buffer."""
        self._sock.settimeout(0.5)
        buf      = b""
        deadline = time.time() + timeout
        while time.time() < deadline:
            try:
                buf += self._sock.recv(4096)
                if buf.decode(errors="replace").strip().count("\n") >= n - 1:
                    break
            except socket.timeout:
                continue
        return buf