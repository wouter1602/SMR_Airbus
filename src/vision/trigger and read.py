#!/usr/bin/env python3
"""
Cognex In-Sight camera – simple Python driver
Connects via ISNM (TCP port 10000)
"""
import numpy as np
import socket
import time
import urllib.request
import io

try:
    from PIL import Image
except ImportError:
    Image = None


class CognexCamera:

    def __init__(self, ip, username="admin", password=""):
        self.ip = ip
        self.username = username
        self.password = password
        self._sock = None

    # ── connection ────────────────────────────────────────────────────

    def connect(self):
        self._sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self._sock.settimeout(5)
        self._sock.connect((self.ip, 23))

        self._read_until(b"User: ")
        self._sock.sendall((self.username + "\r\n").encode())

        after_user = self._read_until(b"Password: ", b"User Logged In", b"Invalid")
        if b"User Logged In" in after_user:
            return

        self._sock.sendall((self.password + "\r\n").encode())
        after_pass = self._read_until(b"User Logged In", b"Invalid Password")

        if b"User Logged In" not in after_pass:
            raise ConnectionError("Login failed")

        # offline → online cycle required after fresh connection
        self._send("SO0")
        time.sleep(0.3)
        self._send("SO1")
        time.sleep(0.8)

    def disconnect(self):
        if self._sock:
            self._sock.close()
            self._sock = None

    # ── camera actions ────────────────────────────────────────────────

    def trigger(self):
        """Fire the camera. Returns True on success."""
        resp = self._send("SW8")
        return resp.strip() == "1"

    def read_cell(self, cell):
        """
        Read a spreadsheet cell value.
        Examples: read_cell("B0"), read_cell("C0"), read_cell("D5")
        """
        cell = cell.strip().upper()
        col = "".join(c for c in cell if c.isalpha())
        row = "".join(c for c in cell if c.isdigit()) or "0"
        command = f"GV{col}{int(row):03d}"

        self._sock.sendall((command + "\r\n").encode())
        raw = self._read_lines(2)
        lines = raw.decode(errors="replace").strip().splitlines()

        if not lines or lines[0].strip() != "1":
            return None                 # cell not found or error
        return lines[1].strip() if len(lines) >= 2 else None

    def trigger_and_read(self, cells):
        """
        Trigger the camera and read one or more cells.
        cells: list of cell names, e.g. ["B0", "C0", "D0"]
        Returns a dict: {"B0": "17.000", "C0": "91829.000", ...}
        """
        self.trigger()
        time.sleep(5.0)
        return {cell: self.read_cell(cell) for cell in cells}

    def set_online(self):
        self._send("SO1")
        time.sleep(0.5)

    def set_offline(self):
        self._send("SO0")

    def get_image(self):
        """
        Fetch the current camera image over HTTP.
        Returns a PIL Image object (requires Pillow: pip install Pillow).
        """
        if Image is None:
            raise ImportError("Install Pillow first:  pip install Pillow")

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
        """Fetch and display the image in a window (requires Pillow)."""
        self.get_image().show()

    # ── internals ─────────────────────────────────────────────────────

    def _send(self, command):
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

    def _read_until(self, *markers, timeout=4.0):
        self._sock.settimeout(0.5)
        buf = b""
        deadline = time.time() + timeout
        while time.time() < deadline:
            try:
                buf += self._sock.recv(4096)
                if any(m in buf for m in markers):
                    break
            except socket.timeout:
                continue
        return buf

    def _read_lines(self, n, timeout=5.0):
        self._sock.settimeout(0.5)
        buf = b""
        deadline = time.time() + timeout
        while time.time() < deadline:
            try:
                buf += self._sock.recv(4096)
                if buf.decode(errors="replace").strip().count("\n") >= n - 1:
                    break
            except socket.timeout:
                continue
        return buf


# ── main ──────────────────────────────────────────────────────────────

if __name__ == "__main__":
    cam = CognexCamera("192.168.0.12", username="admin", password="")
    cam.connect()
    print("Connected")

    # --- trigger ---
    success = cam.trigger()
    print("Trigger:", success)

    cells = ["S38", "S39", "S40", "S41", "S42", "S43"]
    results = cam.trigger_and_read(cells)

    values = np.array([float(results[c]) if results[c] is not None else np.nan for c in cells],dtype=np.float32)
    print(values)

    cam.disconnect()
