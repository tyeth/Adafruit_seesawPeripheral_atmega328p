// Example seesaw peripheral keypad for ATmega328P (Arduino Uno/Nano)
// This creates a keypad peripheral that can be read via I2C
// Compatible with Adafruit_seesaw keypad module in CircuitPython/Arduino

#define PRODUCT_CODE            0x4B50  // "KP" for KeyPad
#define CONFIG_I2C_PERIPH_ADDR  0x49

// Enable UART debugging (optional - uses D0/D1, reducing available keys)
//#define CONFIG_UART_DEBUG       1

// Enable interrupt output
#define CONFIG_INTERRUPT_PIN    2

// Enable keypad support
#define CONFIG_KEYPAD           1

// ATmega328P pin availability (20 GPIO total):
//   Reserved: A4=SDA, A5=SCL (I2C), D2 (IRQ) = 3 pins
//   Reserved if UART debug: D0=RX, D1=TX = 2 more pins
//   Available without UART: D0,D1,D3-D13,A0-A3 = 17 pins
//   Available with UART:    D3-D13,A0-A3 = 15 pins
#ifdef CONFIG_UART_DEBUG
  #define CONFIG_NUM_KEYPAD_KEYS  15
#else
  #define CONFIG_NUM_KEYPAD_KEYS  16
#endif

// Define which pins are used for keypad keys
// Each key is a button connected between the pin and GND (active low with pullup)
#ifdef CONFIG_UART_DEBUG
// 15 keys when UART debug enabled (D0/D1 unavailable)
const uint8_t CONFIG_KEYPAD_PINS[CONFIG_NUM_KEYPAD_KEYS] = {
  3, 4, 5, 6,      // Keys 0-3
  7, 8, 9, 10,     // Keys 4-7
  11, 12, 13,      // Keys 8-10
  14, 15, 16, 17   // Keys 11-14 (A0-A3)
};
#else
// 16 keys when UART debug disabled (D0/D1 available)
const uint8_t CONFIG_KEYPAD_PINS[CONFIG_NUM_KEYPAD_KEYS] = {
  0, 1, 3, 4,      // Keys 0-3 (D0, D1, D3, D4)
  5, 6, 7, 8,      // Keys 4-7
  9, 10, 11, 12,   // Keys 8-11
  13, 14, 15, 16   // Keys 12-15 (D13, A0-A2)
};
#endif

// Note: For real 4x4 matrix keypad, you'd typically use row/column scanning
// with 8 pins (4 rows + 4 columns). This example uses direct GPIO for simplicity.

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
