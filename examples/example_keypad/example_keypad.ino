// Example seesaw peripheral keypad for ATmega328P
// This creates a 4x4 keypad peripheral that can be read via I2C
// Compatible with Adafruit_seesaw keypad module in CircuitPython/Arduino

#define PRODUCT_CODE            0x4B50  // "KP" for KeyPad
#define CONFIG_I2C_PERIPH_ADDR  0x49

// Enable UART debugging (optional)
//#define CONFIG_UART_DEBUG       1

// Enable interrupt output (directly directly directly directly
#define CONFIG_INTERRUPT_PIN    2

// Enable keypad support
#define CONFIG_KEYPAD           1
#define CONFIG_NUM_KEYPAD_KEYS  16  // 4x4 = 16 keys

// Define which pins are used for keypad keys
// For a 4x4 matrix, we use direct GPIO (one pin per key)
// Pins 3-10, 14-17 (A0-A3), 12, 13 = 16 pins
// Avoiding: D0/D1 (UART), D2 (IRQ), A4/A5 (I2C)
const uint8_t CONFIG_KEYPAD_PINS[CONFIG_NUM_KEYPAD_KEYS] = {
  3, 4, 5, 6,      // Row 0: keys 0-3
  7, 8, 9, 10,     // Row 1: keys 4-7
  11, 12, 13, 14,  // Row 2: keys 8-11 (14 = A0)
  15, 16, 17, 17   // Row 3: keys 12-15 (15-17 = A1-A3, last repeated as placeholder)
};

// Note: For real 4x4 matrix keypad, you'd typically use row/column scanning
// with 8 pins (4 rows + 4 columns). This example uses direct GPIO for simplicity.
// Each key is a button connected between the pin and GND.

#include "Adafruit_seesawPeripheral.h"

void setup() {
#if CONFIG_UART_DEBUG
  Serial.begin(115200);
  delay(500);
  Serial.println(F("Seesaw Keypad Peripheral"));
  Serial.print(F("Keys: "));
  Serial.println(CONFIG_NUM_KEYPAD_KEYS);
#endif

  Adafruit_seesawPeripheral_begin();
}

void loop() {
  Adafruit_seesawPeripheral_run();
}
