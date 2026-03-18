"""
test_scan.py
------------
Quick test to verify barcode is being read correctly from Modbus.
Run this while MBTools is running.
"""

from pymodbus.client import ModbusTcpClient

IP   = "168"
PORT = 5020

SCAN_FLAG_REGISTER      = 2
BARCODE_START_REGISTER  = 10
BARCODE_REGISTER_LENGTH = 20

client = ModbusTcpClient(IP, port=PORT)

if not client.connect():
    print("❌ Could not connect to Modbus server")
    exit()

print("✅ Connected to Modbus server")

# --- Read scan flag ---
flag_result = client.read_holding_registers(SCAN_FLAG_REGISTER, count=1)
print(f"\nScan flag (register {SCAN_FLAG_REGISTER}): {flag_result.registers[0]}")

# --- Read raw barcode registers ---
barcode_result = client.read_holding_registers(BARCODE_START_REGISTER, count=BARCODE_REGISTER_LENGTH)
print(f"\nRaw barcode registers {BARCODE_START_REGISTER}–{BARCODE_START_REGISTER + BARCODE_REGISTER_LENGTH - 1}:")
print(f"  {barcode_result.registers}")

# --- Decode barcode ---
barcode = ""
for reg in barcode_result.registers:
    if reg == 0:
        break
    barcode += chr(reg)

print(f"\nDecoded barcode: '{barcode}'")

if not barcode:
    print("⚠️  Barcode is empty — check that you wrote values to registers 10+ in MBTools")
else:
    print("✅ Barcode looks good!")

client.close()
