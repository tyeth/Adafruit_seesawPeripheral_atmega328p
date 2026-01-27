# CircuitPython test code for example_atmega328_gpio seesaw peripheral
# Copy this to code.py on your CircuitPython board
#
# Tested on: Pimoroni Explorer RP2350 (PIM720)
#   - I2C: SDA=GP20, SCL=GP21
#   - Use board.I2C() or board.STEMMA_I2C() if available
#
# Wiring:
#   Pimoroni Explorer     ATmega328P (Uno/Nano)
#   -----------------     ---------------------
#   SDA (GP20)            A4 (pin 18)
#   SCL (GP21)            A5 (pin 19)
#   GND                   GND
#
# Test: Connect ATmega328P pin D5 to GND to see value change

import time
import board
from adafruit_seesaw.seesaw import Seesaw

# I2C address must match CONFIG_I2C_PERIPH_ADDR in the .ino file
SEESAW_ADDR = 0x49

# Test pins on the ATmega328P - avoid D0/D1 (UART), D2 (IRQ), A4/A5 (I2C)
TEST_INPUT_PIN = 5   # D5 - connect to GND to test
TEST_OUTPUT_PIN = 6  # D6 - could connect LED+resistor to GND

print("ATmega328P Seesaw GPIO Test")
print("=" * 40)

# Initialize I2C - use board.I2C() for Pimoroni Explorer RP2350
# This uses the default I2C pins (SDA=GP20, SCL=GP21)
try:
    i2c = board.I2C()
except RuntimeError:
    # Fallback if board.I2C() not available
    import busio
    i2c = busio.I2C(board.SCL, board.SDA)

# Wait for I2C device to be ready
while not i2c.try_lock():
    pass
devices = i2c.scan()
i2c.unlock()
print(f"I2C devices found: {[hex(d) for d in devices]}")

if SEESAW_ADDR not in devices:
    print(f"ERROR: Seesaw not found at {hex(SEESAW_ADDR)}")
    print("Check wiring and ensure ATmega328P is programmed")
    while True:
        time.sleep(1)

# Connect to seesaw
# Note: ATmega328P uses HW_ID 0x87 which matches ATtiny817, so the
# CircuitPython driver will accept it but use ATtiny8x7 pin mapping.
# For basic GPIO testing this doesn't matter since we use raw pin numbers.
ss = Seesaw(i2c, addr=SEESAW_ADDR)

# Read device info
print(f"Chip ID: {hex(ss.chip_id)}")
print(f"Version: {hex(ss.get_version())}")
print(f"Options: {hex(ss.get_options())}")

print()
print("GPIO Test")
print("-" * 40)

# Configure test input pin with pullup
ss.pin_mode(TEST_INPUT_PIN, ss.INPUT_PULLUP)
print(f"Pin D{TEST_INPUT_PIN} configured as INPUT_PULLUP")

# Configure test output pin
ss.pin_mode(TEST_OUTPUT_PIN, ss.OUTPUT)
print(f"Pin D{TEST_OUTPUT_PIN} configured as OUTPUT")

print()
print(f"Connect D{TEST_INPUT_PIN} to GND to see value change")
print(f"D{TEST_OUTPUT_PIN} will toggle every second")
print("Press Ctrl+C to stop")
print()

output_state = False

try:
    while True:
        # Read input pin
        input_val = ss.digital_read(TEST_INPUT_PIN)

        # Toggle output pin
        output_state = not output_state
        ss.digital_write(TEST_OUTPUT_PIN, output_state)

        # Read bulk GPIO (all pins at once)
        bulk_val = ss.digital_read_bulk(0xFFFFFFFF)

        print(f"D{TEST_INPUT_PIN}={input_val}  D{TEST_OUTPUT_PIN}={output_state}  Bulk=0x{bulk_val:08X}")

        time.sleep(1)

except KeyboardInterrupt:
    print("\nTest stopped")
