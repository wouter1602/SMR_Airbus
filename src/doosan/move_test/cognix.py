#!/usr/bin/env python3

import asyncio
import numpy as np
import time
import urllib.request
import io

try:
    from PIL import Image
except ImportError:
    import Image


class CognexCamera:

    def __init__(self, ip, username="admin", password=""):
        self.ip = ip
        self.username = username
        self.password = password
        self._reader = None
        self._writer = None

    # ── connection ────────────────────────────────────────────────────

    async def connect(self):
        self._reader, self._writer = await asyncio.open_connection(self.ip, 23)

        await self._read_until(b"User: ")
        self._writer.write((self.username + "\r\n").encode())
        await self._writer.drain()

        after_user = await self._read_until(b"Password: ", b"User Logged In", b"Invalid")
        if b"User Logged In" in after_user:
            return

        self._writer.write((self.password + "\r\n").encode())
        await self._writer.drain()

        after_pass = await self._read_until(b"User Logged In", b"Invalid Password")
        if b"User Logged In" not in after_pass:
            raise ConnectionError("Login failed")

        # offline → online cycle required after fresh connection
        await self._send("SO0")
        await asyncio.sleep(0.3)
        await self._send("SO1")
        await asyncio.sleep(0.8)

    async def disconnect(self):
        if self._writer:
            self._writer.close()
            await self._writer.wait_closed()
            self._reader = None
            self._writer = None

    # ── camera actions ────────────────────────────────────────────────

    async def trigger(self):
        """Fire the camera. Returns True on success."""
        resp = await self._send("SW8")
        return resp.strip() == "1"

    async def read_cell(self, cell):
        """
        Read a spreadsheet cell value.
        Examples: read_cell("B0"), read_cell("C0"), read_cell("D5")
        """
        cell = cell.strip().upper()
        col = "".join(c for c in cell if c.isalpha())
        row = "".join(c for c in cell if c.isdigit()) or "0"
        command = f"GV{col}{int(row):03d}"

        self._writer.write((command + "\r\n").encode())
        await self._writer.drain()

        raw = await self._read_lines(2)
        lines = raw.decode(errors="replace").strip().splitlines()

        if not lines or lines[0].strip() != "1":
            return None
        return lines[1].strip() if len(lines) >= 2 else None

    async def trigger_and_read(self, cells):
        """
        Trigger the camera and read one or more cells.
        cells: list of cell names, e.g. ["B0", "C0", "D0"]
        Returns a dict: {"B0": "17.000", "C0": "91829.000", ...}
        """
        await self.trigger()
        await asyncio.sleep(5.0)
        return {cell: await self.read_cell(cell) for cell in cells}

    async def set_online(self):
        await self._send("SO1")
        await asyncio.sleep(0.5)

    async def set_offline(self):
        await self._send("SO0")

    async def get_image(self):
        """
        Fetch the current camera image over HTTP.
        Returns a PIL Image object (requires Pillow: pip install Pillow).
        Note: Uses asyncio.to_thread to avoid blocking the event loop.
        """
        if Image is None:
            raise ImportError("Install Pillow first:  pip install Pillow")

        def _fetch():
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

        return await asyncio.to_thread(_fetch)

    async def show_image(self):
        """Fetch and display the image in a window (requires Pillow)."""
        img = await self.get_image()
        img.show()

    # ── internals ─────────────────────────────────────────────────────

    async def _send(self, command):
        self._writer.write((command + "\r\n").encode())
        await self._writer.drain()
        try:
            data = await asyncio.wait_for(self._reader.readline(), timeout=5.0)
        except asyncio.TimeoutError:
            data = b""
        return data.decode(errors="replace").strip()

    async def _read_until(self, *markers, timeout=4.0):
        buf = b""
        try:
            async with asyncio.timeout(timeout):
                while True:
                    chunk = await self._reader.read(4096)
                    if not chunk:
                        break
                    buf += chunk
                    if any(m in buf for m in markers):
                        break
        except asyncio.TimeoutError:
            pass
        return buf

    async def _read_lines(self, n, timeout=5.0):
        buf = b""
        try:
            async with asyncio.timeout(timeout):
                while True:
                    chunk = await self._reader.read(4096)
                    if not chunk:
                        break
                    buf += chunk
                    if buf.decode(errors="replace").strip().count("\n") >= n - 1:
                        break
        except asyncio.TimeoutError:
            pass
        return buf


# ── main ──────────────────────────────────────────────────────────────

async def main():
    cam = CognexCamera("192.168.0.12", username="admin", password="")
    await cam.connect()
    print("Connected")

    success = await cam.trigger()
    print("Trigger:", success)

    cells = ["S38", "S39", "S40", "S41", "S42", "S43"]
    results = await cam.trigger_and_read(cells)

    def to_float(v):
        try:
            return float(v)
        except (TypeError, ValueError):
            return None

    floats = [to_float(results[c]) for c in cells]
    values = None if any(v is None for v in floats) else np.array(floats, dtype=np.float32)
    print(values)

    await cam.disconnect()


if __name__ == "__main__":
    asyncio.run(main())
