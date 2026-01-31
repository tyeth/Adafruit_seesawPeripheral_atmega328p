// Example seesaw peripheral for ATmega328P (Arduino Uno/Nano)
// Matrix Keypad support using Adafruit_Keypad library
// This creates a 3x4 (or 4x4) matrix keypad peripheral readable via I2C
// Compatible with Adafruit_seesaw keypad module in CircuitPython/Arduino
//
// Designed for: Adafruit Membrane 3x4 Matrix Keypad (PID 419)
// https://www.adafruit.com/product/419
//
// Requires: Adafruit_Keypad library (install via Arduino Library Manager)
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

// Enable keypad support
#define CONFIG_KEYPAD           1

// Number of keys in the matrix (3 cols x 4 rows = 12 keys)
#define CONFIG_NUM_KEYPAD_KEYS  12

// IMPORTANT: Do NOT define CONFIG_KEYPAD_PINS - we handle scanning ourselves
// This prevents the built-in direct GPIO scanner from running

// Increase FIFO size for responsive matrix scanning
#define CONFIG_KEYPAD_FIFO_SIZE 16

// Exclude matrix pins (D3-D9) from seesaw GPIO management
// by adding them as "address" pins (which get excluded from VALID_GPIO)
#define CONFIG_ADDR_0           1
#define CONFIG_ADDR_0_PIN       3
#define CONFIG_ADDR_1           1
#define CONFIG_ADDR_1_PIN       4
#define CONFIG_ADDR_2           1
#define CONFIG_ADDR_2_PIN       5
#define CONFIG_ADDR_3           1
#define CONFIG_ADDR_3_PIN       6
// Pins 7,8,9 need to be excluded too - add custom mask after include
#define MATRIX_COL_PINS_MASK    ((1UL << 7) | (1UL << 8) | (1UL << 9))

#include "Adafruit_seesawPeripheral.h"

// --- Matrix Keypad Configuration ---
// For 3x4 membrane keypad (PID 419)
#define ROWS 4
#define COLS 3

// Pin mapping for ATmega328P (Arduino Nano/Uno)
// Keypad pins 1-7 left to right when viewing from front
// Testing: assume pins 1-4 are rows, pins 5-7 are columns (standard layout)
const byte rowPins[ROWS] = {3, 4, 5, 6};   // Keypad pins 1-4 -> D3-D6
const byte colPins[COLS] = {7, 8, 9};      // Keypad pins 5-7 -> D7-D9

// Key map - maps physical position to logical key number (0-11)
// The layout matches a standard telephone keypad:
//   1 2 3     -> keys 0, 1, 2
//   4 5 6     -> keys 3, 4, 5
//   7 8 9     -> keys 6, 7, 8
//   * 0 #     -> keys 9, 10, 11
const byte keyMap[ROWS][COLS] = {
  {0, 1, 2},
  {3, 4, 5},
  {6, 7, 8},
  {9, 10, 11}
};

// Track previous key states for edge detection
uint8_t keyState[ROWS][COLS] = {0};

void setup() {
#ifdef CONFIG_CLOCK_8MHZ
  // Set clock prescaler to divide by 2 (16MHz -> 8MHz)
  // This must be done first, before any timing-dependent code
  // Safe for 3.3V operation per ATmega328P datasheet (0-10MHz @ 2.7V+)
  cli();                  // Disable interrupts
  CLKPR = (1 << CLKPCE);  // Enable prescaler change
  CLKPR = (1 << CLKPS0);  // Set prescaler to 2 (divide by 2)
  sei();                  // Re-enable interrupts
#endif

#if CONFIG_UART_DEBUG
  Serial.begin(115200);
  delay(500);
  Serial.println(F("Seesaw Matrix Keypad Peripheral"));
  Serial.print(F("Keys: "));
  Serial.print(ROWS);
  Serial.print(F("x"));
  Serial.print(COLS);
  Serial.print(F(" = "));
  Serial.println(CONFIG_NUM_KEYPAD_KEYS);
#endif

  // Initialize matrix pins
  // Rows as outputs (directly drive LOW during scan)
  for (byte r = 0; r < ROWS; r++) {
    pinMode(rowPins[r], OUTPUT);
    digitalWrite(rowPins[r], HIGH);  // Idle HIGH
  }
  // Columns as inputs with pullups
  for (byte c = 0; c < COLS; c++) {
    pinMode(colPins[c], INPUT_PULLUP);
  }

#if CONFIG_UART_DEBUG
  Serial.print(F("Row pins: "));
  for (byte r = 0; r < ROWS; r++) { Serial.print(rowPins[r]); Serial.print(F(" ")); }
  Serial.println();
  Serial.print(F("Col pins: "));
  for (byte c = 0; c < COLS; c++) { Serial.print(colPins[c]); Serial.print(F(" ")); }
  Serial.println();

  // Diagnostic: read column pins with all rows HIGH (idle)
  delay(10);
  Serial.print(F("Col states (all rows HIGH): "));
  for (byte c = 0; c < COLS; c++) {
    Serial.print(digitalRead(colPins[c]));
    Serial.print(F(" "));
  }
  Serial.println();

  // Test each row individually
  for (byte r = 0; r < ROWS; r++) {
    digitalWrite(rowPins[r], LOW);
    delayMicroseconds(50);
    Serial.print(F("Row "));
    Serial.print(r);
    Serial.print(F(" LOW, cols: "));
    for (byte c = 0; c < COLS; c++) {
      Serial.print(digitalRead(colPins[c]));
      Serial.print(F(" "));
    }
    Serial.println();
    digitalWrite(rowPins[r], HIGH);
    delay(1);
  }
#endif

  // Initialize seesaw peripheral
  Adafruit_seesawPeripheral_begin();

  // RE-initialize matrix pins AFTER seesaw (seesaw may override our settings)
  for (byte r = 0; r < ROWS; r++) {
    pinMode(rowPins[r], OUTPUT);
    digitalWrite(rowPins[r], HIGH);
  }
  for (byte c = 0; c < COLS; c++) {
    pinMode(colPins[c], INPUT_PULLUP);
  }

#if CONFIG_UART_DEBUG
  Serial.println(F("Matrix pins re-initialized after seesaw"));
#endif
}

void scanMatrix() {
  // Scan each row
  for (byte r = 0; r < ROWS; r++) {
    // Drive this row LOW
    digitalWrite(rowPins[r], LOW);
    delayMicroseconds(10);  // Let signals settle

    // Read each column
    for (byte c = 0; c < COLS; c++) {
      byte pressed = (digitalRead(colPins[c]) == LOW) ? 1 : 0;
      byte keyNum = keyMap[r][c];

      // Check for state change
      if (pressed != keyState[r][c]) {
        keyState[r][c] = pressed;

        if (pressed) {
          Adafruit_seesawPeripheral_keypad_push_event(keyNum, KEYPAD_EDGE_FALLING);
#if CONFIG_UART_DEBUG
          Serial.print(F("Key "));
          Serial.print(keyNum);
          Serial.print(F(" [R"));
          Serial.print(r);
          Serial.print(F("C"));
          Serial.print(c);
          Serial.println(F("] PRESSED"));
#endif
        } else {
          Adafruit_seesawPeripheral_keypad_push_event(keyNum, KEYPAD_EDGE_RISING);
#if CONFIG_UART_DEBUG
          Serial.print(F("Key "));
          Serial.print(keyNum);
          Serial.print(F(" [R"));
          Serial.print(r);
          Serial.print(F("C"));
          Serial.print(c);
          Serial.println(F("] RELEASED"));
#endif
        }
      }
    }

    // Return row to HIGH
    digitalWrite(rowPins[r], HIGH);
  }
}

void loop() {
  // Run seesaw I2C command processor
  Adafruit_seesawPeripheral_run();

  // Scan the matrix keypad every 10ms for debouncing
  static uint32_t lastScan = 0;
  if (millis() - lastScan >= 10) {
    scanMatrix();
    lastScan = millis();
  }
}
