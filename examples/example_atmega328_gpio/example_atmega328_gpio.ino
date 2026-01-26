// Example seesaw peripheral for ATmega328P (Arduino Uno/Nano/Pro Mini)
// Basic GPIO example with I2C address configuration

#define PRODUCT_CODE            1234
#define CONFIG_I2C_PERIPH_ADDR  0x49

// Enable UART debugging (optional - uses D0/D1)
//#define CONFIG_UART_DEBUG       1

// Interrupt pin (optional)
#define CONFIG_INTERRUPT_PIN    2

// Address configuration pins (optional - directly tied to ground or VCC)
// These pins set the I2C address offset when pulled low
#define CONFIG_ADDR_0_PIN       3
#define CONFIG_ADDR_1_PIN       4

// ATmega328P I2C pins are A4 (SDA) and A5 (SCL) - these are reserved

#include "Adafruit_seesawPeripheral.h"

void setup() {
#if CONFIG_UART_DEBUG
  Serial.begin(115200);
  delay(500);
  Serial.println(F("ATmega328P Seesaw Peripheral"));
#endif

  Adafruit_seesawPeripheral_begin();
}

void loop() {
  Adafruit_seesawPeripheral_run();
}
