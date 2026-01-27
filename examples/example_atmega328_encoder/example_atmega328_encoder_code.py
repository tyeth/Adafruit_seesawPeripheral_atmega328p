# CircuitPython test code for example_atmega328_encoder seesaw peripheral
# Copy this to code.py on your CircuitPython board
#
# Tested on: Pimoroni Explorer RP2350 (PIM720)
#
# Wiring:
#   Pimoroni Explorer     ATmega328P (Uno/Nano)
#   -----------------     ---------------------
#   SDA (GP20)            A4 (pin 18)
#   SCL (GP21)            A5 (pin 19)
#   GND                   GND
#
#   Rotary Encoder        ATmega328P
#   --------------        ----------
#   A (CLK)               D5
#   B (DT)                D6
#   C (common)            GND
#   SW (button, optional) D7

import time
import board
from adafruit_seesaw.seesaw import Seesaw
from adafruit_seesaw.rotaryio import IncrementalEncoder

SEESAW_ADDR = 0x49

# Encoder index (0-3 depending on CONFIG_NUM_ENCODERS in firmware)
ENCODER_INDEX = 0

print("ATmega328P Seesaw Encoder Test")
print("=" * 40)

try:
    i2c = board.I2C()
except RuntimeError:
    import busio
    i2c = busio.I2C(board.SCL, board.SDA)

# Wait for I2C device
while not i2c.try_lock():
    pass
devices = i2c.scan()
i2c.unlock()
print(f"I2C devices found: {[hex(d) for d in devices]}")

if SEESAW_ADDR not in devices:
    print(f"ERROR: Seesaw not found at {hex(SEESAW_ADDR)}")
    while True:
        time.sleep(1)

ss = Seesaw(i2c, addr=SEESAW_ADDR)
print(f"Chip ID: {hex(ss.chip_id)}")
print(f"Options: {hex(ss.get_options())}")

print()
print(f"Encoder Test - encoder {ENCODER_INDEX} on D5/D6")
print("-" * 40)

# Create encoder object
encoder = IncrementalEncoder(ss, ENCODER_INDEX)

print("Turn the encoder to see position changes")
print("Press Ctrl+C to stop")
print()

last_position = encoder.position

try:
    while True:
        position = encoder.position

        if position != last_position:
            delta = position - last_position
            direction = "CW" if delta > 0 else "CCW"
            print(f"Position: {position:6d}  Delta: {delta:+3d}  ({direction})")
            last_position = position

        time.sleep(0.01)  # Small delay to prevent flooding

except KeyboardInterrupt:
    print("\nTest stopped")
