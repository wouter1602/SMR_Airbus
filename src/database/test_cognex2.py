"""
test_cognex2.py
---------------
Scans a wide range of registers to find where Cognex
actually writes the barcode and wing ID data.
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

# Read registers 0-124 (Cognex max is 125)
result = client.read_holding_registers(address=0, count=15)
registers = result.registers

def decode_cognex_buffer(registers):
    result = ""
    for reg in registers:
        high = (reg >> 8) & 0xFF
        low = reg & 0xFF

        for byte in (high, low):
            if byte == 0:
                return result
            result += chr(byte)
    return result

decoded = decode_cognex_buffer(registers)
print("Decoded string:", decoded)
if result.isError():
    print("❌ Error reading registers")
    client.close()
    exit()

regs = result.registers

# Print only non-zero registers
print("=== NON-ZERO REGISTERS (0-199) ===")
for i, val in enumerate(regs):
    if val != 0:
        ascii_char = chr(val) if 1 <= val <= 126 else "."
        print(f"  Register {i:3d} = {val:6d}  (ASCII: {ascii_char})")

print()

# Try to find "ABC123" pattern anywhere in the registers
print("=== SEARCHING FOR ABC123 PATTERN ===")
target = [65, 66, 67, 49, 50, 51]  # ABC123
for i in range(len(regs) - 6):
    if list(regs[i:i+6]) == target:
        print(f"  ✅ Found 'ABC123' starting at register {i}!")
        break
else:
    print("  ❌ 'ABC123' not found — checking for any ASCII text sequences...")
    # Find any sequence of printable ASCII characters
    seq_start = None
    seq = ""
    for i, val in enumerate(regs):
        if 32 <= val <= 126:
            if seq_start is None:
                seq_start = i
            seq += chr(val)
        else:
            if len(seq) >= 3:
                print(f"  Register {seq_start}: '{seq}'")
            seq_start = None
            seq = ""

    print()
print("=== TRYING 2 CHARS PER REGISTER DECODE (registers 32-80) ===")
for start in range(0, 160):
    text = ""
    for i in range(32, min(81, len(regs))):
        val = regs[i]
        high = (val >> 8) & 0xFF
        low  = val & 0xFF
        if high == 0 and low == 0:
            break
        if high != 0:
            text += chr(high)
        if low != 0:
            text += chr(low)
    if len(text) >= 3 and text.isprintable():
        print(f"  Register {start}: '{text}'")
client.close()
