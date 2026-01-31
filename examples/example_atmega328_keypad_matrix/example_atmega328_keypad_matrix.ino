// Example seesaw peripheral for ATmega328P (Arduino Uno/Nano)
// Native matrix keypad support via CONFIG_KEYPAD_MATRIX
// This creates a 3x4 (or 4x4) matrix keypad peripheral readable via I2C
// Compatible with Adafruit_seesaw keypad module in CircuitPython/Arduino
//
// Designed for: Adafruit Membrane 3x4 Matrix Keypad (PID 419)
// https://www.adafruit.com/product/419
//
// Wiring for PID 419 (3x4 membrane keypad):
// The keypad has 7 pins (left to right, viewing keypad from front):
//
//   Keypad Pin    Function        ATmega328P Pin
//   ----------    --------        --------------
//   Pin 1         Row 1 (1,2,3)   D3
//   Pin 2         Row 2 (4,5,6)   D4
//   Pin 3         Row 3 (7,8,9)   D5
//   Pin 4         Row 4 (*,0,#)   D6
//   Pin 5         Col 1 (1,4,7,*) D7
//   Pin 6         Col 2 (2,5,8,0) D8
//   Pin 7         Col 3 (3,6,9,#) D9
//
// I2C connection to host (e.g., Pimoroni Explorer):
//   ATmega328P A4 (SDA) -> Host SDA
//   ATmega328P A5 (SCL) -> Host SCL
//   GND -> GND
//
// Key number mapping reported via seesaw:
//   Key 0="1"  Key 1="2"  Key 2="3"
//   Key 3="4"  Key 4="5"  Key 5="6"
//   Key 6="7"  Key 7="8"  Key 8="9"
//   Key 9="*"  Key 10="0" Key 11="#"
//
// 3.3V Operation:
//   To run at 3.3V, enable CONFIG_CLOCK_8MHZ below and compile with:
//   --build-property build.f_cpu=8000000L
//   This uses the clock prescaler to divide 16MHz crystal to 8MHz (safe at 3.3V)

// Uncomment for 3.3V operation at 8MHz (requires build flag, see above)
// #define CONFIG_CLOCK_8MHZ       1

#define PRODUCT_CODE            0x4B4D  // "KM" for Keypad Matrix
#define CONFIG_I2C_PERIPH_ADDR  0x49

// Enable UART debugging (optional - uses D0/D1)
#define CONFIG_UART_DEBUG       1

// Enable interrupt output on D2
#define CONFIG_INTERRUPT_PIN    2

// ============ Matrix Keypad Configuration ============
// Enable matrix keypad support (uses row/column scanning)
#define CONFIG_KEYPAD           1
#define CONFIG_KEYPAD_MATRIX    1

// Matrix dimensions (3 columns x 4 rows = 12 keys)
#define CONFIG_KEYPAD_ROWS      4
#define CONFIG_KEYPAD_COLS      3

// Row pins (directly define the array before including library)
const uint8_t CONFIG_KEYPAD_ROW_PINS[CONFIG_KEYPAD_ROWS] = {3, 4, 5, 6};   // D3-D6
const uint8_t CONFIG_KEYPAD_COL_PINS[CONFIG_KEYPAD_COLS] = {7, 8, 9};      // D7-D9

// Increase FIFO size for responsive matrix scanning
#define CONFIG_KEYPAD_FIFO_SIZE 16

#include "Adafruit_seesawPeripheral.h"

void setup() {
#ifdef CONFIG_CLOCK_8MHZ
  // Set clock prescaler to divide by 2 (16MHz -> 8MHz)
  // Safe for 3.3V operation per ATmega328P datasheet (0-10MHz @ 2.7V+)
  cli();
  CLKPR = (1 << CLKPCE);
  CLKPR = (1 << CLKPS0);
  sei();
#endif

#if CONFIG_UART_DEBUG
  Serial.begin(115200);
  delay(500);
  Serial.println(F("Seesaw Matrix Keypad Peripheral"));
  Serial.print(F("Matrix: "));
  Serial.print(CONFIG_KEYPAD_ROWS);
  Serial.print(F("x"));
  Serial.print(CONFIG_KEYPAD_COLS);
  Serial.print(F(" = "));
  Serial.println(CONFIG_NUM_KEYPAD_KEYS);
#endif

  // Initialize seesaw peripheral (matrix pins are initialized at end of begin())
  Adafruit_seesawPeripheral_begin();

#if CONFIG_UART_DEBUG
  Serial.println(F("Ready"));
#endif
}

void loop() {
  // Run seesaw I2C command processor (includes periodic keypad scanning)
  Adafruit_seesawPeripheral_run();
}
