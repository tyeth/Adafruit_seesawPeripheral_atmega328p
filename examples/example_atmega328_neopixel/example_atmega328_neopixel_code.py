# CircuitPython test code for example_atmega328_neopixel seesaw peripheral
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
#   NeoPixel Strip        ATmega328P
#   -------------         ----------
#   DIN                   D5 (or any GPIO, configured below)
#   VCC                   5V
#   GND                   GND

import time
import board
from adafruit_seesaw.seesaw import Seesaw
from adafruit_seesaw.neopixel import NeoPixel

SEESAW_ADDR = 0x49

# NeoPixel configuration
NEOPIXEL_PIN = 5      # ATmega328P pin for NeoPixel data (D5)
NUM_PIXELS = 8        # Number of NeoPixels in the strip
BRIGHTNESS = 0.3      # Brightness (0.0 to 1.0)

print("ATmega328P Seesaw NeoPixel Test")
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
print(f"NeoPixel Test - {NUM_PIXELS} pixels on pin D{NEOPIXEL_PIN}")
print("-" * 40)

# Create NeoPixel object through seesaw
pixels = NeoPixel(ss, NEOPIXEL_PIN, NUM_PIXELS, brightness=BRIGHTNESS, auto_write=False)

# Color definitions
RED = (255, 0, 0)
GREEN = (0, 255, 0)
BLUE = (0, 0, 255)
WHITE = (255, 255, 255)
OFF = (0, 0, 0)

def wheel(pos):
    """Generate rainbow colors across 0-255 positions."""
    if pos < 85:
        return (255 - pos * 3, pos * 3, 0)
    elif pos < 170:
        pos -= 85
        return (0, 255 - pos * 3, pos * 3)
    else:
        pos -= 170
        return (pos * 3, 0, 255 - pos * 3)

def rainbow_cycle(wait):
    """Rainbow cycle animation."""
    for j in range(256):
        for i in range(NUM_PIXELS):
            rc_index = (i * 256 // NUM_PIXELS) + j
            pixels[i] = wheel(rc_index & 255)
        pixels.show()
        time.sleep(wait)

print("Running color test...")
print("Press Ctrl+C to stop")
print()

try:
    while True:
        # Solid colors
        print("Red")
        pixels.fill(RED)
        pixels.show()
        time.sleep(1)

        print("Green")
        pixels.fill(GREEN)
        pixels.show()
        time.sleep(1)

        print("Blue")
        pixels.fill(BLUE)
        pixels.show()
        time.sleep(1)

        # Rainbow cycle
        print("Rainbow...")
        rainbow_cycle(0.01)

except KeyboardInterrupt:
    print("\nTurning off pixels")
    pixels.fill(OFF)
    pixels.show()
    print("Test stopped")
