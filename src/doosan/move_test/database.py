#!/usr/bin/env python3

import os
from openpyxl import Workbook, load_workbook
from datetime import datetime
from typing import Tuple, Any
import asyncio

FILE_NAME = "wing_panel.xlsx"

POSITIONS = [
    "1-1", "1-2", "1-3", "1-4", "1-5", "1-6", "1-7", "1-8",
    "2-1", "2-2", "2-3", "2-4", "2-5", "2-6", "2-7", "2-8",
]


class WingPanelDB:
    def __init__(self, file_name: str = FILE_NAME):
        self.file_name = file_name
        self._current_row: int | None = None
        self._current_wing: str | None = None

    def _load_sheet(self) -> Tuple[Workbook, Any]:
        if os.path.exists(self.file_name):
            workbook = load_workbook(self.file_name)
        else:
            workbook = Workbook()
            sheet = workbook.active or workbook.create_sheet()
            sheet.append(["WING ID", "Timestamp"] + POSITIONS)

        return workbook, workbook.active

    async def start_wing(self, wing_id: str):
        await asyncio.to_thread(self._start_wing, wing_id)

    def _start_wing(self, wing_id: str):
        workbook, sheet = self._load_sheet()

        timestamp = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
        sheet.append([wing_id, timestamp] + [""] * len(POSITIONS))

        self._current_row = sheet.max_row
        self._current_wing = wing_id

        workbook.save(self.file_name)

    async def add_panel(self, position: str, barcode: str):
        await asyncio.to_thread(self._add_panel, position, barcode)

    def _add_panel(self, position: str, barcode: str) -> None:
        if self._current_row is None:
            raise RuntimeError("No active wing. Call start_wing() first.")

        if position not in POSITIONS:
            raise ValueError(f"Invalid position: {position!r}. Must be one of {POSITIONS}.")

        workbook, sheet = self._load_sheet()

        col_index = POSITIONS.index(position) + 3  # offset: Wing ID + Timestamp
        sheet.cell(row=self._current_row, column=col_index).value = barcode

        workbook.save(self.file_name)

    @property

    def current_wing(self) -> str | None:
        return self._current_wing
