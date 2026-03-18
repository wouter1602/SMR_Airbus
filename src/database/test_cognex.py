"""
test_cognex.py
--------------
Reads all relevant registers from the Cognex scanner
and prints a full dump so we can see what's coming through.

Run this while the Cognex is connected and trigger a scan,
then run it again to see what changed.
"""

from pymodbus.client import ModbusTcpClient

IP   = "192.168.0.10"
PORT = 502

client = ModbusTcpClient(IP, port=PORT)

if not client.connect():
    print("❌ Could not connect to Cognex")
    exit()

print("✅ Connected to Cognex at 192.168.0.10:502")
print()

# Read registers 0-40 to see everything
result = client.read_holding_registers(0, count=40)

if result.isError():
    print("❌ Error reading registers")
    client.close()
    exit()

regs = result.registers
print("=== RAW REGISTER DUMP (0-39) ===")
for i, val in enumerate(regs):
    print(f"  Register {i:2d} = {val:5d}  (ASCII: {chr(val) if 32 <= val <= 126 else '.' })")

print()

# Try to decode barcode from register 1
print("=== BARCODE (registers 1-20) ===")
barcode = ""
for i in range(1, 21):
    if regs[i] == 0:
        break
    barcode += chr(regs[i])
print(f"  Decoded: '{barcode}'")

print()

# Try to decode wing ID from register 21
print("=== WING ID (registers 21-29) ===")
wing = ""
for i in range(21, 30):
    if regs[i] == 0:
        break
    wing += chr(regs[i])
print(f"  Decoded: '{wing}'")

print()
print(f"=== INSPECTION (register 0) ===")
print(f"  Value: {regs[0]} → {'PASS' if regs[0] == 1 else 'FAIL/EMPTY'}")

client.close()
print()
print("Done. Trigger a scan in Cognex and run this script again to see values change.")
