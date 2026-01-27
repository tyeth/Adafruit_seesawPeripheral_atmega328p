// Example seesaw peripheral for ATmega328P - NeoPixel
// Allows controlling NeoPixel strips via I2C

#define PRODUCT_CODE            1234
#define CONFIG_I2C_PERIPH_ADDR  0x49

// Enable UART debugging (optional - uses D0/D1)
//#define CONFIG_UART_DEBUG       1

// Interrupt pin (optional)
//#define CONFIG_INTERRUPT_PIN    2

// Address configuration pins
#define CONFIG_ADDR_0_PIN       3
#define CONFIG_ADDR_1_PIN       4

// Enable NeoPixel support
// The NeoPixel data pin is configured by the host via I2C
// Buffer size limits max number of pixels (3 bytes per pixel for GRB)
#define CONFIG_NEOPIXEL         1
#define CONFIG_NEOPIXEL_BUF_MAX 180  // 60 pixels * 3 bytes = 180

// ATmega328P I2C pins are A4 (SDA) and A5 (SCL) - these are reserved

#include "Adafruit_seesawPeripheral.h"

void setup() {
#if CONFIG_UART_DEBUG
  Serial.begin(115200);
  delay(500);
  Serial.println(F("ATmega328P Seesaw NeoPixel Peripheral"));
  Serial.print(F("Max buffer: "));
  Serial.println(CONFIG_NEOPIXEL_BUF_MAX);
#endif

  Adafruit_seesawPeripheral_begin();
}

void loop() {
  Adafruit_seesawPeripheral_run();
}
