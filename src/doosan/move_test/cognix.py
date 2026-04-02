#!/usr/bin/env python3

import asyncio
import numpy as np
import urllib.request
import io
import logging

from typing import Any
from enum import IntEnum

try:
    from PIL import Image
except ImportError:
    import Image

TRIGGER_WAIT_BOX = 1.0
TRIGGER_WAIT_CELL = 2.0

logger = logging.getLogger(__name__)

class PickupType(IntEnum):
    Empty = 0
    Paper = 1
    Foam = 2
    No_detect = 3
    Wrong_Panel = 4
    Correct_panel = 5

class CognexCamera:

    def __init__(self, ip: str, username: str ="admin", password: str = "", port: int = 23):
        self.ip: str = ip
        self.port: int = port
        self.username: str = username
        self.password: str = password
        self._reader: asyncio.StreamReader | None = None
        self._writer: asyncio.StreamWriter | None = None

    # ── connection ────────────────────────────────────────────────────

    async def connect(self) -> None:
        self._reader, self._writer = await asyncio.open_connection(self.ip, self.port)

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

    async def trigger(self)-> bool:
        """Fire the camera. Returns True on success."""
        resp = await self._send("SW8")
        return resp.strip() == "1"

    async def read_cell(self, cell: str) -> str | None:
        """
        Read a spreadsheet cell value.
        Examples: read_cell("B0"), read_cell("C0"), read_cell("D5")
        """
        cell = cell.strip().upper()
        col = "".join(c for c in cell if c.isalpha())
        row = "".join(c for c in cell if c.isdigit()) or "0"
        command = f"GV{col}{int(row):03d}"
        if self._writer is not None:
            self._writer.write((command + "\r\n").encode())
            await self._writer.drain()
        else:
            raise RuntimeError("Not connected")

        raw = await self._read_lines(2)
        lines = raw.decode(errors="replace").strip().splitlines()

        if not lines or lines[0].strip() != "1":
            return None
        return lines[1].strip() if len(lines) >= 2 else None

    async def _trigger_and_read(self, cells: list[str], wait: float) -> dict[str, str | None] | None:
        await self.trigger()
        await asyncio.sleep(wait)

        try:
            result = {cell: await self.read_cell(cell) for cell in cells}
        except Exception as e:
            logger.error(f"trigger_and_read failed: {e}")
            return None

        for cell, val in result.items():
            if val is None:
                logger.warning(f"read_cell({cell}) returned None")
        return result

    async def set_online(self) -> None:
        await self._send("SO1")
        await asyncio.sleep(0.5)

    async def set_offline(self) -> None:
        await self._send("SO0")

    async def get_image(self) -> Image.Image | None:
        """
        Fetch the current camera image over HTTP.
        Returns a PIL Image object (requires Pillow: pip install Pillow).
        Note: Uses asyncio.to_thread to avoid blocking the event loop.
        """
        if Image is None:
            raise ImportError("Install Pillow first:  pip install Pillow")

        def _fetch() -> Image.Image:
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

    async def show_image(self) -> None:
        """Fetch and display the image in a window (requires Pillow)."""
        img = await self.get_image()
        if img is not None:
            img.show()

    # ── internals ────────────────────────────────────────────────────-

    async def _send(self, command: str) -> str:
        if self._writer is None:
            raise RuntimeError("Not connected")

        if self._reader is None:
            raise RuntimeError("Not connected")


        self._writer.write((command + "\r\n").encode())
        await self._writer.drain()
        try:
            data = await asyncio.wait_for(self._reader.readline(), timeout=5.0)
        except asyncio.TimeoutError:
            data = b""
        return data.decode(errors="replace").strip()

    async def _read_until(self, *markers, timeout: float = 4.0) -> bytes:
        if self._reader is None:
            raise RuntimeError("Not connected")

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

    async def _read_lines(self, n, timeout: float=5.0) -> bytes:
        if self._reader is None:
            raise RuntimeError("Not connected")
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
        except Exception as e:
            logger.error(f"_read_lines failed: {e}")
        return buf

class CameraTCP(CognexCamera):
    async def change_job_box(self) -> None:
        await self._send("SIA0261")
        await asyncio.sleep(0.8)

    async def change_job_scan(self) -> None:
        await self._send("SIA0260")
        await asyncio.sleep(0.8)

    async def scan_box(self, type: int) -> PickupType:

        def _convert(value: str | None) -> bool:

            if value == '#ERR':
                return False
            elif value is None:
                return False
            try:
                return True if float(value) == 1.0 else False
            except (ValueError, TypeError):
                return False
            except Exception as e:
                logger.error(f"_convert failed: {e}")
                return False

        position_cells = ["A42", "B42", "C42", "D42", "E42", "F42", "G42"]

        # read right cells
        await self.change_job_box()

        scan_box_result = await self._trigger_and_read(position_cells, TRIGGER_WAIT_BOX)

        if scan_box_result is None:
            return PickupType.No_detect

        # convert result
        paper = _convert(scan_box_result['A42'])
        panel_M = _convert(scan_box_result['B42'])
        found = _convert(scan_box_result['C42'])
        panel = _convert(scan_box_result['D42'])
        empty = _convert(scan_box_result['E42'])
        foam = _convert(scan_box_result['F42'])
        left_gap = _convert(scan_box_result['G42'])

        # decide output #TODO: rewrite this if statement nesting
        if empty:
            return PickupType.Empty
        elif paper:
            return PickupType.Paper
        elif foam:
            return PickupType.Foam
        elif panel:
            if type == 1:
                await self.change_job_scan()
                position_cells_scan = ["Q35"]
                data = await self._trigger_and_read(position_cells_scan, TRIGGER_WAIT_CELL)
                if data is None:
                    return PickupType.No_detect
                else:
                    if isinstance(data['Q35'], float):
                        lenght = int(float(data['Q35']))
                    else:
                        return PickupType.Correct_panel
                    if 325<lenght<370:

                        return PickupType.Correct_panel
                    else:
                        return PickupType.Wrong_Panel

            elif type == 2:
                if found:
                    if panel_M:
                        if not left_gap:
                            return PickupType.Correct_panel
                        else:
                            return PickupType.Wrong_Panel
                    else:
                        return PickupType.Wrong_Panel
                else:
                    return PickupType.No_detect
            elif type == 3:
                if found:
                    if panel_M:
                        if left_gap:
                            return PickupType.Correct_panel
                        else:
                            return PickupType.Wrong_Panel
                    else:
                        return PickupType.Wrong_Panel
                else:
                    return PickupType.No_detect
        else:
            return PickupType.No_detect

        return PickupType.No_detect

    async def scan_scan(self, type: int) -> np.ndarray:
        await self.change_job_scan()

        if type == 1:
            position_cells = ["U38", "U39", "U40", "U41", "U42", "U43"]
        elif type == 2:
            position_cells = ["T38", "T39", "T40", "T41", "T42", "T43"]
        elif type == 3:
            position_cells = ["S38", "S39", "S40", "S41", "S42", "S43"]
        else:
            raise ValueError(f"Invalid scan type: {type}")

        data = await self._trigger_and_read(position_cells, TRIGGER_WAIT_CELL)

        if data is None:
            return np.array([np.nan] * len(position_cells), dtype=np.float32)

        def to_float(v: str | None) -> float | None:
            if v is None:
                return None
            try:
                return float(v)
            except ValueError:
                return None

        values = np.array([to_float(data[c]) if data[c] is not None else np.nan for c in position_cells], dtype=np.float32)

        return values

class CameraCode(CognexCamera):
    async def scan_qr(self) -> str | None:
        data = await self._trigger_and_read(["C2"], TRIGGER_WAIT_CELL)

        if data is None:
            return None

        if data["C2"] is None:
            return None

        if data["C2"] == '#ERR':
            return None

        return data["C2"]

    async def scan_barcode(self) -> str | None:
        data = await self._trigger_and_read(["C4"], TRIGGER_WAIT_CELL)

        if data is None:
            return None

        if data["C4"] is None:
            return None

        if data["C4"] == '#ERR':
            return None

        return data["C4"]

# ── main ──────────────────────────────────────────────────────────────

async def main():
    cam = CognexCamera("192.168.0.12", username="admin", password="")
    await cam.connect()
    print("Connected")

    success = await cam.trigger()
    print("Trigger:", success)

    cells = ["S38", "S39", "S40", "S41", "S42", "S43"]

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
