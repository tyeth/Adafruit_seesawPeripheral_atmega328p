# CircuitPython test code for example_atmega328_pwm seesaw peripheral
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
# Test: Connect an LED (with resistor) to a PWM pin to see brightness change
#   LED+  ---[220R]--- D9 (or D3, D5, D6, D10, D11)
#   LED-  ------------ GND

import time
import board
from adafruit_seesaw.seesaw import Seesaw
from adafruit_seesaw.pwmout import PWMOut

SEESAW_ADDR = 0x49

# PWM pins on ATmega328P: D3, D5, D6, D9, D10, D11
# Using D9 for this test (Timer1, 16-bit)
PWM_PIN = 9

print("ATmega328P Seesaw PWM Test")
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
print(f"PWM Test - fading LED on D{PWM_PIN}")
print("-" * 40)

# Create PWM output on the specified pin
pwm = PWMOut(ss, PWM_PIN)

# PWM frequency (optional - not all seesaw firmwares support this)
try:
    pwm.frequency = 1000  # 1kHz
    print(f"PWM frequency set to {pwm.frequency}Hz")
except Exception as e:
    print(f"Could not set frequency: {e}")

print()
print("Fading LED up and down...")
print("Press Ctrl+C to stop")
print()

try:
    while True:
        # Fade up
        for duty in range(0, 65536, 1024):
            pwm.duty_cycle = duty
            time.sleep(0.02)

        # Fade down
        for duty in range(65535, -1, -1024):
            pwm.duty_cycle = duty
            time.sleep(0.02)

except KeyboardInterrupt:
    pwm.duty_cycle = 0
    print("\nTest stopped")
