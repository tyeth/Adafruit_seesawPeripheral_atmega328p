# CircuitPython test code for example_keypad seesaw peripheral
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
# Keypad buttons on ATmega328P (directly connect button between pin and GND):
#
# With UART debug DISABLED (16 keys):
#   Key 0: D0      Key 4: D5      Key 8:  D9      Key 12: D13
#   Key 1: D1      Key 5: D6      Key 9:  D10     Key 13: A0 (D14)
#   Key 2: D3      Key 6: D7      Key 10: D11     Key 14: A1 (D15)
#   Key 3: D4      Key 7: D8      Key 11: D12     Key 15: A2 (D16)
#
# With UART debug ENABLED (15 keys, D0/D1 unavailable):
#   Key 0: D3      Key 4: D7      Key 8:  D11     Key 11: A0 (D14)
#   Key 1: D4      Key 5: D8      Key 9:  D12     Key 12: A1 (D15)
#   Key 2: D5      Key 6: D9      Key 10: D13     Key 13: A2 (D16)
#   Key 3: D6      Key 7: D10                     Key 14: A3 (D17)

import time
import board
from adafruit_seesaw.keypad import Keypad, KeyEvent

SEESAW_ADDR = 0x49

# Number of keys depends on firmware config:
# - 16 keys if UART debug disabled (default)
# - 15 keys if UART debug enabled
NUM_KEYS = 16  # Adjust to 15 if firmware has CONFIG_UART_DEBUG enabled

# Edge types
EDGE_NAMES = {
    Keypad.EDGE_HIGH: "HIGH",
    Keypad.EDGE_LOW: "LOW",
    Keypad.EDGE_FALLING: "FALLING",
    Keypad.EDGE_RISING: "RISING",
}

print("ATmega328P Seesaw Keypad Test")
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

# Create Keypad object (subclass of Seesaw)
keypad = Keypad(i2c, addr=SEESAW_ADDR)
print(f"Chip ID: {hex(keypad.chip_id)}")
print(f"Options: {hex(keypad.get_options())}")

print()
print(f"Keypad Test - {NUM_KEYS} keys")
print("-" * 40)

# Enable events for all keys
# We'll track FALLING (key pressed) and RISING (key released) events
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

try:
    while True:
        # Check how many events are in the FIFO
        count = keypad.count

        if count > 0:
            # Read all pending events (each event is 1 byte)
            # Upper 6 bits = key number, lower 2 bits = edge type
            events = keypad.read_keypad(count)

            for event_byte in events:
                # Decode the event
                key_num = (event_byte >> 2) & 0x3F
                edge = event_byte & 0x03
                edge_name = EDGE_NAMES.get(edge, f"UNKNOWN({edge})")

                # Create KeyEvent for cleaner access
                event = KeyEvent(key_num, edge)

                action = "PRESSED" if edge == Keypad.EDGE_FALLING else "RELEASED"
                print(f"Key {event.number:2d}: {action:8s} (edge={edge_name})")

        time.sleep(0.01)  # Small delay to prevent flooding

except KeyboardInterrupt:
    # Disable interrupt
    keypad.interrupt_enabled = False
    print("\nTest stopped")
