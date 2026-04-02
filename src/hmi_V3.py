import tkinter as tk
from datetime import datetime
import queue
import threading
import sys
import numpy as np
import socket
import time
import urllib.request
import io
import os
from enum import IntEnum
from openpyxl import Workbook, load_workbook

try:
    from PIL import Image, ImageTk
except ImportError:
    Image = None
    ImageTk = None


# ==============================================================================
# STANDALONE send_command — use this anywhere you have an HMI reference
# ==============================================================================
def send_command(hmi, command: str):
    """
    Thread-safe command sender. Can be called from any thread — camera callbacks,
    PLC loops, console input, timers, etc.

    Supported commands:
        STATUS  <STOPPED|WARNING|RUNNING|FINISHED|HUMAN NEEDED>
        PLACED  <row>-<col>           e.g.  send_command(app, "PLACED 1-3")
        PLACE   <panel> AT <r,c>      e.g.  send_command(app, "PLACE PNL1 AT 0,2")
        BARCODE <code>                e.g.  send_command(app, "BARCODE XYZ999")
        RESET                         e.g.  send_command(app, "RESET")

    Examples:
        send_command(app, "STATUS WARNING")
        send_command(app, "STATUS RUNNING")
        send_command(app, "STATUS HUMAN NEEDED")
        send_command(app, "PLACED 2-5")
        send_command(app, "BARCODE ABC123")
        send_command(app, "RESET")
    """
    if not isinstance(command, str) or not command.strip():
        print(f"[send_command] Invalid command: {repr(command)}")
        return
    hmi.cmd_queue.put(command.strip())


class SolarHMI:
    def __init__(self, root, cam1=None, cam2=None):
        self.root = root
        self.root.title("Solar Panel Placement HMI")
        self.root.geometry("1000x800")

        self.cam1 = cam1
        self.cam2 = cam2

        self.grid_state = [[None for _ in range(8)] for _ in range(2)]
        self.log_data = []

        # Thread-safe command queue
        self.cmd_queue = queue.Queue()

        self.create_widgets()
        self.poll_commands()

    # --------------------------------------------------------------------------
    # INSTANCE METHOD — call as app.send_command("STATUS RUNNING")
    # --------------------------------------------------------------------------
    def send_command(self, command: str):
        """
        Instance-level wrapper around the queue. Thread-safe; works from any thread.

        Supported commands:
            STATUS  <STOPPED|WARNING|RUNNING|FINISHED|HUMAN NEEDED>
            PLACED  <row>-<col>
            PLACE   <panel> AT <r,c>
            BARCODE <code>
            RESET

        Examples (from camera callback, PLC thread, anywhere):
            self.send_command("STATUS WARNING")
            self.send_command("PLACED 1-4")
            self.send_command("BARCODE XYZ001")
            self.send_command("RESET")
        """
        if not isinstance(command, str) or not command.strip():
            print(f"[send_command] Invalid command: {repr(command)}")
            return
        self.cmd_queue.put(command.strip())

    def create_widgets(self):
        # --- Top Status Frame ---
        top_frame = tk.Frame(self.root)
        top_frame.pack(pady=10)

        self.status_labels = {}
        for status in ["STOPPED", "WARNING", "RUNNING", "FINISHED", "HUMAN NEEDED"]:
            lbl = tk.Label(top_frame, text=status, width=12, bg="gray", fg="white")
            lbl.pack(side=tk.LEFT, padx=5)
            self.status_labels[status] = lbl

        # --- Grid Frame ---
        grid_frame = tk.Frame(self.root)
        grid_frame.pack(pady=10)

        self.grid_buttons = []
        for r in range(2):
            row = []
            for c in range(8):
                btn = tk.Button(
                    grid_frame,
                    text=f"{r+1}-{c+1}",
                    width=8,
                    height=2,
                    command=lambda r=r, c=c: self.add_log(f"Grid click {r+1}-{c+1}")
                )
                btn.grid(row=r, column=c, padx=2, pady=2)
                row.append(btn)
            self.grid_buttons.append(row)

        # --- Camera Image Frame ---
        cam_frame = tk.Frame(self.root)
        cam_frame.pack(pady=10)

        self.image_label = tk.Label(cam_frame, text="No Image", bg="black", fg="white", width=56, height=15)
        self.image_label.pack(side=tk.LEFT, padx=10)

        cam_btn_frame = tk.Frame(cam_frame)
        cam_btn_frame.pack(side=tk.LEFT, padx=10)

        tk.Button(cam_btn_frame, text="Snap & Show Cam 1", command=lambda: self.request_image(1)).pack(pady=5, fill=tk.X)
        tk.Button(cam_btn_frame, text="Snap & Show Cam 2", command=lambda: self.request_image(2)).pack(pady=5, fill=tk.X)

        # --- Control & Log Frame ---
        control_frame = tk.Frame(self.root)
        control_frame.pack(pady=10)

        tk.Button(control_frame, text="Start System", command=self.start_system).pack(side=tk.LEFT, padx=5)
        tk.Button(control_frame, text="Stop System", command=self.stop_system).pack(side=tk.LEFT, padx=5)

        self.log_box = tk.Text(self.root, height=8)
        self.log_box.pack(fill=tk.BOTH, expand=True, padx=10, pady=10)

    def set_status(self, status):
        colors = {
            "STOPPED": "red",
            "WARNING": "orange",
            "RUNNING": "green",
            "FINISHED": "blue",
            "HUMAN NEEDED": "purple"
        }
        for s in self.status_labels:
            self.status_labels[s].config(bg="gray")
        if status in self.status_labels:
            self.status_labels[status].config(bg=colors.get(status, "gray"))

    def start_system(self):
        self.add_log("System started")
        self.set_status("RUNNING")

    def stop_system(self):
        self.add_log("System stopped")
        self.set_status("STOPPED")

    def add_log(self, message):
        timestamp = datetime.now().strftime("%H:%M:%S")
        entry = f"[{timestamp}] {message}\n"
        self.log_box.insert(tk.END, entry)
        self.log_box.see(tk.END)
        self.log_data.append(entry)
        print(f"HMI Action: {message}")

    # --- Image Handling Methods ---
    def request_image(self, cam_num):
        self.add_log(f"Requesting image from Camera {cam_num}...")
        self.image_label.config(text="Fetching...", image="")
        threading.Thread(target=self._fetch_image_task, args=(cam_num,), daemon=True).start()

    def _fetch_image_task(self, cam_num):
        cam = self.cam1 if cam_num == 1 else self.cam2
        if not cam:
            self.root.after(0, self.add_log, f"Camera {cam_num} is not initialized/connected!")
            self.root.after(0, lambda: self.image_label.config(text="Camera Error"))
            return
        try:
            cam.trigger()
            img = cam.get_image()
            img = img.resize((400, 300), Image.LANCZOS)
            self.root.after(0, self.update_image_ui, img)
        except Exception as e:
            self.root.after(0, self.add_log, f"Error fetching image: {e}")
            self.root.after(0, lambda: self.image_label.config(text="Fetch Failed"))

    def update_image_ui(self, pil_image):
        if ImageTk is None:
            self.add_log("Error: Pillow (ImageTk) is not installed.")
            return
        self.current_photo = ImageTk.PhotoImage(pil_image)
        self.image_label.config(image=self.current_photo, text="", width=400, height=300)
        self.add_log("Image updated.")

    # SAFE: called only from main thread via poll_commands
    def handle_command(self, cmd):
        parts = cmd.strip().split()
        if not parts:
            return

        if parts[0] == "STATUS":
            status = " ".join(parts[1:]).upper()
            self.set_status(status)
            self.add_log(f"Status changed to {status}")

        elif parts[0] == "PLACE":
            panel = parts[1]
            r, c = map(int, parts[3].split(","))
            self.grid_state[r][c] = panel
            self.grid_buttons[r][c].config(text=panel)
            self.add_log(f"Auto: {cmd}")

        elif parts[0] == "PLACED":
            r_user, c_user = map(int, parts[1].split("-"))
            r, c = r_user - 1, c_user - 1
            if 0 <= r < 2 and 0 <= c < 8:
                self.grid_state[r][c] = "AUTO"
                self.grid_buttons[r][c].config(text="AUTO")
                self.add_log(f"Auto placed at {r_user},{c_user}")
            else:
                self.add_log(f"Invalid position: {parts[1]}")

        elif parts[0] == "BARCODE":
            self.add_log(f"Barcode scanned: {parts[1]}")

        elif parts[0] == "RESET":
            for r in range(2):
                for c in range(8):
                    self.grid_state[r][c] = None
                    self.grid_buttons[r][c].config(text=f"{r+1}-{c+1}")
            self.add_log("Grid reset")

    def poll_commands(self):
        try:
            while True:
                cmd = self.cmd_queue.get_nowait()
                self.handle_command(cmd)
        except queue.Empty:
            pass
        self.root.after(100, self.poll_commands)


# ==============================================================================
# CAMERA DRIVER
# ==============================================================================
class CognexCamera:
    def __init__(self, ip, username="admin", password=""):
        self.ip = ip
        self.username = username
        self.password = password
        self._sock = None

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

        self._send("SO0")
        time.sleep(0.3)
        self._send("SO1")
        time.sleep(0.8)
        print(f'Connected to {self.ip}')

    def disconnect(self):
        if self._sock:
            self._sock.close()
            self._sock = None

    def trigger(self):
        resp = self._send("SW8")
        time.sleep(0.3)
        return resp.strip() == "1"

    def read_cell(self, cell):
        cell = cell.strip().upper()
        col = "".join(c for c in cell if c.isalpha())
        row = "".join(c for c in cell if c.isdigit()) or "0"
        command = f"GV{col}{int(row):03d}"
        self._sock.sendall((command + "\r\n").encode())
        raw = self._read_lines(2)
        lines = raw.decode(errors="replace").strip().splitlines()
        if not lines or lines[0].strip() != "1":
            return None
        return lines[1].strip() if len(lines) >= 2 else None

    def trigger_and_read_box(self, cells):
        self.trigger()
        time.sleep(1.0)
        return {cell: self.read_cell(cell) for cell in cells}

    def trigger_and_read_scan(self, cells):
        self.trigger()
        time.sleep(2.0)
        return {cell: self.read_cell(cell) for cell in cells}

    def set_online(self):
        self._send("SO1")
        time.sleep(0.5)

    def set_offline(self):
        self._send("SO0")

    def get_image(self):
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

    def change_job_box(self):
        self._send("SIA0261")

    def change_job_scan(self):
        self._send("SIA0260")

    def show_image(self):
        self.get_image().show()

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

    def set_job_by_id(self, job_id):
        if not isinstance(job_id, int):
            raise ValueError("job_id must be an integer")
        if job_id < 0 or job_id > 999:
            raise ValueError("job_id must be between 0 and 999")
        self.set_offline()
        time.sleep(0.5)
        self._sock.sendall((f"SJ{job_id}\r\n").encode())
        response = self._read_lines(1, timeout=3).decode(errors="replace").strip()
        print(f"SJ response raw: '{response}'")
        if not response:
            raise RuntimeError("No response from camera (timeout)")
        try:
            status = int(response)
        except ValueError:
            raise RuntimeError(f"Unexpected response from camera: {response}")
        if status != 1:
            raise RuntimeError(f"SJ failed with status {status}")
        return True


class PickupType(IntEnum):
    Empty = 0
    Paper = 1
    Foam = 2
    No_detect = 3
    wrong_panel = 4
    right_panel = 5


# ==============================================================================
# CONSOLE LOOP
# ==============================================================================
def console_loop(app):
    """
    Lets you type commands directly in the terminal while the GUI is running.
    All commands are forwarded via send_command() so they're thread-safe.

    Example commands to try:
        STATUS WARNING
        STATUS RUNNING
        STATUS HUMAN NEEDED
        PLACED 1-3
        BARCODE XYZ001
        RESET
        EXIT
    """
    print("\nConsole ready. Type a command and press Enter.")
    print("  STATUS WARNING | STATUS RUNNING | STATUS HUMAN NEEDED")
    print("  PLACED 1-3     | BARCODE XYZ001 | RESET | EXIT\n")

    while True:
        line = sys.stdin.readline()
        if not line:
            break
        cmd = line.strip()
        if cmd.upper() == "EXIT":
            break
        if cmd:
            send_command(app, cmd)  # <-- uses the standalone function


# ==============================================================================
# MAIN EXECUTION
# ==============================================================================
if __name__ == "__main__":
    # 1. Initialize Cameras
    try:
        cam1 = CognexCamera("192.168.0.12", username="admin", password="")
        cam1.connect()
    except Exception as e:
        print(f"Warning: Failed to connect to Cam1: {e}")
        cam1 = None

    try:
        cam2 = CognexCamera("192.168.0.10", username="admin", password="")
        cam2.connect()
    except Exception as e:
        print(f"Warning: Failed to connect to Cam2: {e}")
        cam2 = None

    # 2. Build GUI
    root = tk.Tk()
    app = SolarHMI(root, cam1, cam2)

    # 3. Start console input thread (non-blocking)
    threading.Thread(target=console_loop, args=(app,), daemon=True).start()
    def my_logic():
        time.sleep(1)                        # wait for window
        app.send_command("STATUS RUNNING")   # fires instantly
    
        time.sleep(2)
        app.send_command("PLACED 1-3") 
        # 2 seconds later

    # loops, camera reads, if/else — whatever you need, no restrictions
    threading.Thread(target=my_logic, daemon=True).start()

    # 4. Run GUI (blocks until window is closed)
    root.mainloop()

    # 5. Clean shutdown
    if cam1:
        cam1.disconnect()
    if cam2:
        cam2.disconnect()

    print("Clean shutdown complete.")