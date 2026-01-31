# CircuitPython test code for example_atmega328_keypad_matrix seesaw peripheral
# Copy this to code.py on your CircuitPython board
#
# Tested on: Pimoroni Explorer RP2350 (PIM720)
# Designed for: Adafruit Membrane 3x4 Matrix Keypad (PID 419)
# https://www.adafruit.com/product/419
#
# Wiring:
#   Pimoroni Explorer     ATmega328P (Nano)     3x4 Keypad
#   -----------------     -----------------     ----------
#   SDA (GP20)            A4 (pin 27)
#   SCL (GP21)            A5 (pin 28)
#   GND                   GND                   (common)
#   3V3 or 5V             VCC
#                         D3                    Row 1 (pin 1)
#                         D4                    Row 2 (pin 2)
#                         D5                    Row 3 (pin 3)
#                         D6                    Row 4 (pin 4)
#                         D7                    Col 1 (pin 5)
#                         D8                    Col 2 (pin 6)
#                         D9                    Col 3 (pin 7)
#
# 3x4 Keypad pinout (7 pins, left to right when viewing from front):
#   Pin 1: Row 1 (top row: 1, 2, 3)
#   Pin 2: Row 2 (second row: 4, 5, 6)
#   Pin 3: Row 3 (third row: 7, 8, 9)
#   Pin 4: Row 4 (bottom row: *, 0, #)
#   Pin 5: Column 1 (left column: 1, 4, 7, *)
#   Pin 6: Column 2 (middle column: 2, 5, 8, 0)
#   Pin 7: Column 3 (right column: 3, 6, 9, #)
#
# Key number mapping (as reported by seesaw):
#   Key 0 = "1"    Key 1 = "2"    Key 2 = "3"
#   Key 3 = "4"    Key 4 = "5"    Key 5 = "6"
#   Key 6 = "7"    Key 7 = "8"    Key 8 = "9"
#   Key 9 = "*"    Key 10 = "0"   Key 11 = "#"

import time
import board
from adafruit_seesaw.keypad import Keypad, KeyEvent

SEESAW_ADDR = 0x49
NUM_KEYS = 12  # 3x4 matrix

# Key labels for display
KEY_LABELS = ["1", "2", "3", "4", "5", "6", "7", "8", "9", "*", "0", "#"]

# Edge types
EDGE_NAMES = {
    Keypad.EDGE_HIGH: "HIGH",
    Keypad.EDGE_LOW: "LOW",
    Keypad.EDGE_FALLING: "FALLING",
    Keypad.EDGE_RISING: "RISING",
}

print("ATmega328P Seesaw Matrix Keypad Test")
print("3x4 Membrane Keypad (PID 419)")
print("=" * 40)

# Set up I2C
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
    print("Check wiring and ensure ATmega328P is programmed")
    while True:
        time.sleep(1)

# Create Keypad object (subclass of Seesaw)
keypad = Keypad(i2c, addr=SEESAW_ADDR)
print(f"Chip ID: {hex(keypad.chip_id)}")
print(f"Options: {hex(keypad.get_options())}")

print()
print(f"Matrix Keypad Test - {NUM_KEYS} keys (3x4)")
print("-" * 40)

# Print keypad layout
print("Keypad layout:")
print("  +---+---+---+")
print("  | 1 | 2 | 3 |  <- Row 1")
print("  +---+---+---+")
print("  | 4 | 5 | 6 |  <- Row 2")
print("  +---+---+---+")
print("  | 7 | 8 | 9 |  <- Row 3")
print("  +---+---+---+")
print("  | * | 0 | # |  <- Row 4")
print("  +---+---+---+")
print()

# Enable events for all keys
print("Configuring key events...")
for key in range(NUM_KEYS):
    # Enable falling edge (key press) detection
    keypad.set_event(key, Keypad.EDGE_FALLING, True)
    # Enable rising edge (key release) detection
    keypad.set_event(key, Keypad.EDGE_RISING, True)

# Enable interrupt (optional - for hardware interrupt pin)
keypad.interrupt_enabled = True

print(f"Keys configured: {keypad.count}")
print()
print("Press keys to see events")
print("Press Ctrl+C to stop")
print()

# Track key states for display
key_states = [False] * NUM_KEYS

try:
    while True:
        # Check how many events are in the FIFO
        count = keypad.count

        if count > 0:
            # Read all pending events
            events = keypad.read_keypad(count)

            for event_byte in events:
                # Decode the event
                key_num = (event_byte >> 2) & 0x3F
                edge = event_byte & 0x03
                edge_name = EDGE_NAMES.get(edge, f"UNKNOWN({edge})")

                if key_num < NUM_KEYS:
                    key_label = KEY_LABELS[key_num]

                    if edge == Keypad.EDGE_FALLING:
                        action = "PRESSED "
                        key_states[key_num] = True
                    else:
                        action = "RELEASED"
                        key_states[key_num] = False

                    print(f"Key {key_num:2d} '{key_label}': {action} (edge={edge_name})")

        time.sleep(0.01)  # Small delay to prevent flooding

except KeyboardInterrupt:
    # Disable interrupt
    keypad.interrupt_enabled = False
    print("\nTest stopped")
