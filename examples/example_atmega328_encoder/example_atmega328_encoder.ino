// Example seesaw peripheral for ATmega328P - Rotary Encoder
// Provides quadrature encoder input with position tracking via I2C
//
// ATmega328P external interrupt pins: D2 (INT0), D3 (INT1)
// For best encoder performance, use these interrupt-capable pins

#define PRODUCT_CODE            1234
#define CONFIG_I2C_PERIPH_ADDR  0x49

// Enable UART debugging (optional - uses D0/D1)
//#define CONFIG_UART_DEBUG       1

// Interrupt pin for notifying host of encoder changes
#define CONFIG_INTERRUPT_PIN    2

// Address configuration pins
#define CONFIG_ADDR_0_PIN       7
#define CONFIG_ADDR_1_PIN       8

// Enable basic features
#define CONFIG_ADC              1
#define CONFIG_PWM              1
#define CONFIG_NEOPIXEL         1
#define CONFIG_NEOPIXEL_BUF_MAX (8*3)  // 8 pixels for encoder ring

// Enable encoder support
#define CONFIG_ENCODER          1
#define CONFIG_NUM_ENCODERS     1

// Encoder pin configuration
// Using D5/D6 for encoder A/B (avoiding D2 interrupt, D0/D1 UART)
#define CONFIG_ENCODER0_A_PIN   5
#define CONFIG_ENCODER0_B_PIN   6

// Uncomment for additional encoders if needed:
//#define CONFIG_ENCODER1_A_PIN   9
//#define CONFIG_ENCODER1_B_PIN   10

// ATmega328P I2C pins are A4 (SDA) and A5 (SCL) - these are reserved

#include "Adafruit_seesawPeripheral.h"

void setup() {
#if CONFIG_UART_DEBUG
  Serial.begin(115200);
  delay(500);
  Serial.println(F("ATmega328P Seesaw Encoder Peripheral"));
  Serial.print(F("Encoder on pins D"));
  Serial.print(CONFIG_ENCODER0_A_PIN);
  Serial.print(F("/D"));
  Serial.println(CONFIG_ENCODER0_B_PIN);
#endif

  Adafruit_seesawPeripheral_begin();
}

void loop() {
  Adafruit_seesawPeripheral_run();
}
