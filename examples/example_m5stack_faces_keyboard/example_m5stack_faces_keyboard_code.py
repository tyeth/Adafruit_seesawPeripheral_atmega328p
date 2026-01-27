# CircuitPython test code for M5Stack FACES Keyboard with seesaw firmware
# Copy this to code.py on your CircuitPython board
#
# Tested on: Pimoroni Explorer RP2350 (PIM720)
#
# Wiring:
#   Pimoroni Explorer     M5Stack FACES Keyboard
#   -----------------     ----------------------
#   SDA (GP20)            SDA (Grove connector or pins)
#   SCL (GP21)            SCL (Grove connector or pins)
#   GND                   GND
#   3.3V or 5V            VCC
#
# The FACES keyboard connects via I2C. If using the M5Stack base,
# it connects through the FACES bottom connector.

import time
import board
from adafruit_seesaw.keypad import Keypad, KeyEvent

# I2C address - must match firmware CONFIG_I2C_PERIPH_ADDR
# Default is 0x5E (different from original M5Stack 0x08)
SEESAW_ADDR = 0x5E

# Key count: 35 matrix keys + 5 modifier keys = 40
NUM_KEYS = 40

# Key index to name mapping
KEY_NAMES = {
    0: 'Q', 1: 'W', 2: 'E', 3: 'R', 4: 'T',
    5: 'Y', 6: 'U', 7: 'I', 8: 'O', 9: 'P',
    10: 'A', 11: 'S', 12: 'D', 13: 'F', 14: 'G',
    15: 'H', 16: 'J', 17: 'K', 18: 'L', 19: 'DEL',
    20: 'ALT', 21: 'Z', 22: 'X', 23: 'C', 24: 'V',
    25: 'B', 26: 'N', 27: 'M', 28: '$', 29: 'ENTER',
    30: 'aA', 31: '0', 32: 'SPACE', 33: 'SYM', 34: 'FN',
    # Modifier keys (duplicate reports for tracking state)
    35: 'MOD_SHIFT', 36: 'MOD_ALT', 37: 'MOD_ENTER',
    38: 'MOD_SYM', 39: 'MOD_FN',
}

# ASCII mapping for keys (lowercase mode)
KEY_ASCII = {
    0: 'q', 1: 'w', 2: 'e', 3: 'r', 4: 't',
    5: 'y', 6: 'u', 7: 'i', 8: 'o', 9: 'p',
    10: 'a', 11: 's', 12: 'd', 13: 'f', 14: 'g',
    15: 'h', 16: 'j', 17: 'k', 18: 'l', 19: '\x08',  # backspace
    21: 'z', 22: 'x', 23: 'c', 24: 'v',
    25: 'b', 26: 'n', 27: 'm', 28: '$', 29: '\r',  # enter
    31: '0', 32: ' ',
}

# Edge types
EDGE_NAMES = {
    Keypad.EDGE_HIGH: "HIGH",
    Keypad.EDGE_LOW: "LOW",
    Keypad.EDGE_FALLING: "PRESSED",
    Keypad.EDGE_RISING: "RELEASED",
}

print("M5Stack FACES Keyboard Seesaw Test")
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
    print(f"ERROR: Keyboard not found at {hex(SEESAW_ADDR)}")
    print("Check wiring and ensure FACES keyboard has seesaw firmware")
    while True:
        time.sleep(1)

# Create Keypad object
keypad = Keypad(i2c, addr=SEESAW_ADDR)
print(f"Chip ID: {hex(keypad.chip_id)}")
print(f"Options: {hex(keypad.get_options())}")

print()
print("Configuring key events...")

# Enable FALLING (press) and RISING (release) events for all keys
for key in range(NUM_KEYS):
    keypad.set_event(key, Keypad.EDGE_FALLING, True)
    keypad.set_event(key, Keypad.EDGE_RISING, True)

# Enable interrupt
keypad.interrupt_enabled = True

print(f"Keys configured: {NUM_KEYS}")
print()
print("Press keys on the FACES keyboard")
print("Press Ctrl+C to stop")
print()

# Track modifier state
modifiers = {
    'shift': False,
    'alt': False,
    'sym': False,
    'fn': False,
}

# Buffer for typed text
text_buffer = ""

try:
    while True:
        count = keypad.count

        if count > 0:
            events = keypad.read_keypad(count)

            for event_byte in events:
                key_num = (event_byte >> 2) & 0x3F
                edge = event_byte & 0x03

                key_name = KEY_NAMES.get(key_num, f"KEY_{key_num}")
                edge_name = EDGE_NAMES.get(edge, f"EDGE_{edge}")

                # Update modifier state
                if edge == Keypad.EDGE_FALLING:  # Key pressed
                    if key_num in (30, 35):  # aA / MOD_SHIFT
                        modifiers['shift'] = not modifiers['shift']
                    elif key_num in (20, 36):  # ALT / MOD_ALT
                        modifiers['alt'] = True
                    elif key_num in (33, 38):  # SYM / MOD_SYM
                        modifiers['sym'] = True
                    elif key_num in (34, 39):  # FN / MOD_FN
                        modifiers['fn'] = True
                    else:
                        # Regular key - add to buffer
                        if key_num in KEY_ASCII:
                            char = KEY_ASCII[key_num]
                            if modifiers['shift'] and char.isalpha():
                                char = char.upper()
                            if char == '\x08' and text_buffer:  # Backspace
                                text_buffer = text_buffer[:-1]
                            elif char == '\r':  # Enter
                                print(f"TEXT: {text_buffer}")
                                text_buffer = ""
                            elif char >= ' ':
                                text_buffer += char

                elif edge == Keypad.EDGE_RISING:  # Key released
                    if key_num in (20, 36):  # ALT
                        modifiers['alt'] = False
                    elif key_num in (33, 38):  # SYM
                        modifiers['sym'] = False
                    elif key_num in (34, 39):  # FN
                        modifiers['fn'] = False

                # Print event
                mod_str = ""
                if modifiers['shift']:
                    mod_str += "[SHIFT]"
                if modifiers['alt']:
                    mod_str += "[ALT]"
                if modifiers['sym']:
                    mod_str += "[SYM]"
                if modifiers['fn']:
                    mod_str += "[FN]"

                print(f"{key_name:12s} {edge_name:8s} {mod_str}  Buffer: {text_buffer}")

        time.sleep(0.01)

except KeyboardInterrupt:
    keypad.interrupt_enabled = False
    print("\nTest stopped")
    if text_buffer:
        print(f"Final buffer: {text_buffer}")
