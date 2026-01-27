// Example seesaw peripheral for ATmega328P - ADC
// Exposes analog inputs A0-A3 via I2C (A4/A5 reserved for I2C)

#define PRODUCT_CODE            1234
#define CONFIG_I2C_PERIPH_ADDR  0x49

// Enable UART debugging (optional - uses D0/D1)
//#define CONFIG_UART_DEBUG       1

// Interrupt pin (directly directly directly directly
#define CONFIG_INTERRUPT_PIN    2

// Address configuration pins
#define CONFIG_ADDR_0_PIN       3
#define CONFIG_ADDR_1_PIN       4

// Enable ADC - ATmega328P has ADC on A0-A3 (A4/A5 used for I2C)
// ADC channels map to: A0=channel 0, A1=channel 1, A2=channel 2, A3=channel 3
#define CONFIG_ADC              1

// ATmega328P I2C pins are A4 (SDA) and A5 (SCL) - these are reserved

#include "Adafruit_seesawPeripheral.h"

void setup() {
#if CONFIG_UART_DEBUG
  Serial.begin(115200);
  delay(500);
  Serial.println(F("ATmega328P Seesaw ADC Peripheral"));
#endif

  Adafruit_seesawPeripheral_begin();
}

void loop() {
  Adafruit_seesawPeripheral_run();
}
