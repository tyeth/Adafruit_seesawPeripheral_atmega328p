// Example seesaw peripheral for ATmega328P - PWM
// Provides PWM outputs controllable via I2C
//
// ATmega328P PWM pins:
//   Timer0: D5 (OC0B), D6 (OC0A) - 8-bit, ~980Hz
//   Timer1: D9 (OC1A), D10 (OC1B) - 16-bit, configurable
//   Timer2: D3 (OC2B), D11 (OC2A) - 8-bit, ~490Hz

#define PRODUCT_CODE            1234
#define CONFIG_I2C_PERIPH_ADDR  0x49

// Enable UART debugging (optional - uses D0/D1)
//#define CONFIG_UART_DEBUG       1

// Interrupt pin (optional)
//#define CONFIG_INTERRUPT_PIN    2

// Address configuration pins
#define CONFIG_ADDR_0_PIN       3
#define CONFIG_ADDR_1_PIN       4

// Enable PWM support
// PWM available on D3, D5, D6, D9, D10, D11
#define CONFIG_PWM              1

// ATmega328P I2C pins are A4 (SDA) and A5 (SCL) - these are reserved

#include "Adafruit_seesawPeripheral.h"

void setup() {
#if CONFIG_UART_DEBUG
  Serial.begin(115200);
  delay(500);
  Serial.println(F("ATmega328P Seesaw PWM Peripheral"));
  Serial.println(F("PWM pins: D3, D5, D6, D9, D10, D11"));
#endif

  Adafruit_seesawPeripheral_begin();
}

void loop() {
  Adafruit_seesawPeripheral_run();
}
