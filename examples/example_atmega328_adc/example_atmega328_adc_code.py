# CircuitPython test code for example_atmega328_adc seesaw peripheral
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
# Test: Connect a potentiometer to A0, or just touch A0 to see noise

import time
import board
from adafruit_seesaw.seesaw import Seesaw
from adafruit_seesaw.analoginput import AnalogInput

SEESAW_ADDR = 0x49

# ADC channels on ATmega328P: A0-A3 (A4/A5 used for I2C)
# Channel numbers: A0=14, A1=15, A2=16, A3=17 (Arduino pin numbers)
ADC_PINS = [14, 15, 16, 17]  # A0, A1, A2, A3

print("ATmega328P Seesaw ADC Test")
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
print("ADC Test - reading A0-A3")
print("-" * 40)

# Create AnalogInput objects for each ADC pin
analog_inputs = []
for pin in ADC_PINS:
    try:
        ai = AnalogInput(ss, pin)
        analog_inputs.append((pin, ai))
        print(f"A{pin-14} (pin {pin}) initialized")
    except Exception as e:
        print(f"A{pin-14} (pin {pin}) error: {e}")

print()
print("Reading ADC values (0-1023 raw, 0-65535 scaled)")
print("Press Ctrl+C to stop")
print()

try:
    while True:
        values = []
        for pin, ai in analog_inputs:
            # value property returns 16-bit scaled value (0-65535)
            val = ai.value
            values.append(f"A{pin-14}={val:5d}")
        print("  ".join(values))
        time.sleep(0.5)

except KeyboardInterrupt:
    print("\nTest stopped")
