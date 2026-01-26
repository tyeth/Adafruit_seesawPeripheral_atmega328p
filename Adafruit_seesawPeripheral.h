/*!
 *  @file Adafruit_seesawPeripheral.h
 */

#ifndef _ADAFRUIT_SEESAWPERIPHERAL_H
#define _ADAFRUIT_SEESAWPERIPHERAL_H

#include "Adafruit_seesaw.h"
#include "Arduino.h"
#include <Wire.h>
#include "wiring_private.h"
#include "pins_arduino.h"

void foo(void);

// Detect ATmega328P/ATmega328PB/ATmega328 chips (Arduino Uno, Nano, Pro Mini, etc.)
#if defined(__AVR_ATmega328P__) || defined(__AVR_ATmega328PB__) || \
    defined(__AVR_ATmega328__) || defined(ARDUINO_AVR_UNO) || \
    defined(ARDUINO_AVR_NANO) || defined(ARDUINO_AVR_PRO) || \
    defined(ARDUINO_AVR_MINI)
#define ATMEGA328_SERIES 1
#endif

// NeoPixel support - timing code works for both MegaTinyCore and ATmega328 at supported speeds
#if CONFIG_NEOPIXEL
  #if defined(MEGATINYCORE) || defined(ATMEGA328_SERIES)
    #include "Adafruit_seesawPeripheral_tinyneopixel.h"
  #endif
#endif

// Original ATtiny-only NeoPixel include (commented out - now unified above):
// #if CONFIG_NEOPIXEL && defined(MEGATINYCORE)
// #include "Adafruit_seesawPeripheral_tinyneopixel.h"
// #endif

#if !defined(CONFIG_EEPROM)
#define CONFIG_EEPROM 1
#endif

#if CONFIG_EEPROM
#include <EEPROM.h>
#define EEPROM_I2C_ADDR (EEPROM.length() - 1)
#endif

/*************** UART debugging */
#if !defined(CONFIG_UART_DEBUG)
#define CONFIG_UART_DEBUG 0
#endif

#if (CONFIG_UART_DEBUG == 1)
#define SEESAW_DEBUG(...) Serial.print(__VA_ARGS__)
#define SEESAW_DEBUGLN(...) Serial.println(__VA_ARGS__)
#elif (CONFIG_UART_DEBUG == 0)
#define SEESAW_DEBUG(...)
#define SEESAW_DEBUGLN(...)
#else
#error("CONFIG_UART_DEBUG must be 0 or 1")
#endif

/*************** Interrupt Pin */
#if defined(CONFIG_INTERRUPT_PIN)
#define CONFIG_INTERRUPT 1
#else
#define CONFIG_INTERRUPT 0
#define CONFIG_INTERRUPT_PIN 0
#endif
#if defined(USE_PINCHANGE_INTERRUPT)
#define USE_PINCHANGE_INTERRUPT 1
#else
#define USE_PINCHANGE_INTERRUPT 0
#endif

/******** FHT (audio spectrum) */
// FHT is ONLY supported on megaTinyCore (AVR), and will only fit if NO OTHER
// seesaw variants are enabled (i.e. NO GPIO or ADC at same time). Mostly due
// to flash space and/or RAM (literally zero overhead on ATtiny816/817), but
// also because ADC for audio-in requires free-run mode which takes exclusive
// use of the ADC MUX anyway. Input pin is currently #defined here, not
// passed over via Seesaw lib. That might be possible if needed, but since
// there's zero RAM remaining, might have to rely on dirty pool like using
// one of the other ADC registers not in use (TEMP or CTRLE) as a temporary
// holding spot to get that value into Adafruit_seesawPeripheral_reset().
// Also FYI, this depends on the FHT library which is NOT available via the
// Arduino Library Manager and must be separately installed.
#if CONFIG_FHT && defined(MEGATINYCORE)
#if CONFIG_ADC
#error("Cannot enable both CONFIG_ADC and CONFIG_FHT")
#endif
// Currently set up for size 128 FHT (64 spectrum outputs). 256 (128 out)
// is an option IF a larger chip (1K RAM or better) is used; won't fit on
// smaller parts. For now, is set for small part...
#define FHT_N 128
#define LOG_OUT 1
#include <FHT.h>
#define FHT_DEFAULT_PIN 0 // Arduino pin # for input (if no channel select)
#define DISABLE_MILLIS    // FHT is exclusive (no GPIO, etc.), can do this
#endif

uint16_t DATE_CODE = 0;

#define CONFIG_VERSION                                                         \
  (uint32_t)(((uint32_t)PRODUCT_CODE << 16) |                                  \
             ((uint16_t)DATE_CODE & 0x0000FFFF))

/********************** Hardcoded chip configuration */

// ATmega328P/U UART debug pins (Arduino D0/D1 = PD0/PD1)
#if defined(ATMEGA328_SERIES)
#define UART_DEBUG_RXD 0
#define UART_DEBUG_TXD 1
#endif

// ATtiny x17/x07 series UART debug pins
#if defined(ARDUINO_AVR_ATtiny817) || defined(ARDUINO_AVR_ATtiny807) ||        \
    defined(ARDUINO_AVR_ATtiny1617) || defined(ARDUINO_AVR_ATtiny1607) ||      \
    defined(ARDUINO_AVR_ATtiny427) || defined(ARDUINO_AVR_ATtiny827) ||      \
    defined(ARDUINO_AVR_ATtiny3217)
#define UART_DEBUG_RXD 8
#define UART_DEBUG_TXD 9
#endif

// ATtiny x16/x06 series UART debug pins (0/1-series and 2-series)
#if defined(ARDUINO_AVR_ATtiny816) || defined(ARDUINO_AVR_ATtiny806) ||        \
    defined(ARDUINO_AVR_ATtiny1616) || defined(ARDUINO_AVR_ATtiny1606) ||      \
    defined(ARDUINO_AVR_ATtiny3216) || defined(ARDUINO_AVR_ATtiny416) ||       \
    defined(ARDUINO_AVR_ATtiny826) || defined(ARDUINO_AVR_ATtiny426) ||        \
    defined(ARDUINO_AVR_ATtiny1626) || defined(ARDUINO_AVR_ATtiny3226)
#define UART_DEBUG_RXD 6
#define UART_DEBUG_TXD 7
#endif

// ATtiny x27/x07 series UART debug pins (2-series)
#if defined(ARDUINO_AVR_ATtiny827) || defined(ARDUINO_AVR_ATtiny427) ||        \
    defined(ARDUINO_AVR_ATtiny1627) || defined(ARDUINO_AVR_ATtiny3227)
#ifndef UART_DEBUG_RXD
#define UART_DEBUG_RXD 8
#define UART_DEBUG_TXD 9
#endif
#endif

// Fallback UART debug pins for MEGATINYCORE if not defined above
#if defined(MEGATINYCORE) && !defined(UART_DEBUG_RXD)
#define UART_DEBUG_RXD 6
#define UART_DEBUG_TXD 7
#endif

#ifdef CONFIG_ADDR_INVERTED
  #undef CONFIG_ADDR_INVERTED
  #define CONFIG_ADDR_INVERTED 1
#else
  #define CONFIG_ADDR_INVERTED 0
#endif

#ifdef CONFIG_ADDR_0_PIN
#define CONFIG_ADDR_0 1
#else
#define CONFIG_ADDR_0 0
#define CONFIG_ADDR_0_PIN 0
#endif
#ifdef CONFIG_ADDR_1_PIN
#define CONFIG_ADDR_1 1
#else
#define CONFIG_ADDR_1 0
#define CONFIG_ADDR_1_PIN 0
#endif
#ifdef CONFIG_ADDR_2_PIN
#define CONFIG_ADDR_2 1
#else
#define CONFIG_ADDR_2 0
#define CONFIG_ADDR_2_PIN 0
#endif
#ifdef CONFIG_ADDR_3_PIN
  #define CONFIG_ADDR_3 1
#else
  #define CONFIG_ADDR_3 0
  #define CONFIG_ADDR_3_PIN 0
#endif

/********************** Available/taken GPIO configuration macros */

// ATmega328P/U GPIO configuration
// Arduino pins: D0-D13 (pins 0-13), A0-A5 (pins 14-19) = 20 total GPIO
// I2C: A4=SDA (pin 18), A5=SCL (pin 19)
// PWM 8-bit: D3, D5, D6, D9, D10, D11 (pins 3, 5, 6, 9, 10, 11)
// PWM 16-bit (Timer1): D9, D10 (pins 9, 10)
// ADC: A0-A5 (pins 14-19)
#if defined(ATMEGA328_SERIES)
#define ALL_GPIO 0x000FFFFFUL  // 20 GPIO pins available (0-19)
#define ALL_ADC ((1UL << 14) | (1UL << 15) | (1UL << 16) | (1UL << 17) | \
                 (1UL << 18) | (1UL << 19))  // A0-A5 have ADC
#ifdef CONFIG_PWM_16BIT
// Timer1 16-bit PWM on D9 (OC1A) and D10 (OC1B)
#define ALL_PWM ((1UL << 9) | (1UL << 10))
#else
// Standard 8-bit PWM pins: D3, D5, D6, D9, D10, D11
#define ALL_PWM ((1UL << 3) | (1UL << 5) | (1UL << 6) | (1UL << 9) | \
                 (1UL << 10) | (1UL << 11))
#endif
#define PWM_WO_OFFSET (9)  // Timer1 OC1A is on pin 9
#endif

// ATtiny x17/x07 series GPIO configuration
#if defined(ARDUINO_AVR_ATtiny817) || defined(ARDUINO_AVR_ATtiny807) ||        \
    defined(ARDUINO_AVR_ATtiny1617) || defined(ARDUINO_AVR_ATtiny1607) ||      \
    defined(ARDUINO_AVR_ATtiny427) || defined(ARDUINO_AVR_ATtiny827) ||      \
    defined(ARDUINO_AVR_ATtiny3217)
#define ALL_GPIO                                                               \
  0x1FFFFFUL // this is chip dependant, for 817 we have 21 GPIO avail (0~20 inc)
#define ALL_ADC 0b1111000000110011001111 // pins that have ADC capability
#ifdef CONFIG_PWM_16BIT
#define ALL_PWM ((1UL << 6) | (1UL << 7) | (1UL << 8))  // alternate TCA0 WOx
#else
#define ALL_PWM                                                                \
  ((1UL << 0) | (1UL << 1) | (1UL << 9) | (1UL << 10) | (1UL << 11) |          \
   (1UL << 12) | (1UL << 13) | (1UL << 10))
#endif
#define PWM_WO_OFFSET (6)
#endif

// ATtiny x16/x06 series GPIO configuration (0/1-series)
#if defined(ARDUINO_AVR_ATtiny816) || defined(ARDUINO_AVR_ATtiny806) ||        \
    defined(ARDUINO_AVR_ATtiny1616) || defined(ARDUINO_AVR_ATtiny1606) ||      \
    defined(ARDUINO_AVR_ATtiny3216) || defined(ARDUINO_AVR_ATtiny416)
#define ALL_GPIO                                                               \
  0x01FFFFUL // this is chip dependant, for 816 we have 17 GPIO avail
#define ALL_ADC 0b11100001100111111 // pins that have ADC capability
#ifdef CONFIG_PWM_16BIT
#define ALL_PWM ((1UL << 4) | (1UL << 5) | (1UL << 6))  // alternate TCA0 WOx
#else
#define ALL_PWM                                                                \
  ((1UL << 0) | (1UL << 1) | (1UL << 7) | (1UL << 8) | (1UL << 9) |            \
   (1UL << 10) | (1UL << 11) | (1UL << 16))
#endif
#define PWM_WO_OFFSET (4)
#endif

// ATtiny x26/x06 series GPIO configuration (2-series) - similar to x16 pinout
#if defined(ARDUINO_AVR_ATtiny826) || defined(ARDUINO_AVR_ATtiny426) ||        \
    defined(ARDUINO_AVR_ATtiny1626) || defined(ARDUINO_AVR_ATtiny3226)
#define ALL_GPIO 0x01FFFFUL  // 17 GPIO like x16
#define ALL_ADC 0b11100001100111111  // similar ADC pins
#ifdef CONFIG_PWM_16BIT
#define ALL_PWM ((1UL << 4) | (1UL << 5) | (1UL << 6))
#else
#define ALL_PWM                                                                \
  ((1UL << 0) | (1UL << 1) | (1UL << 7) | (1UL << 8) | (1UL << 9) |            \
   (1UL << 10) | (1UL << 11) | (1UL << 16))
#endif
#define PWM_WO_OFFSET (4)
#endif

// ATtiny x27/x07 series GPIO configuration (2-series) - similar to x17 pinout
#if defined(ARDUINO_AVR_ATtiny827) || defined(ARDUINO_AVR_ATtiny427) ||        \
    defined(ARDUINO_AVR_ATtiny1627) || defined(ARDUINO_AVR_ATtiny3227)
#define ALL_GPIO 0x1FFFFFUL  // 21 GPIO like x17
#define ALL_ADC 0b1111000000110011001111  // similar ADC pins
#ifdef CONFIG_PWM_16BIT
#define ALL_PWM ((1UL << 6) | (1UL << 7) | (1UL << 8))
#else
#define ALL_PWM                                                                \
  ((1UL << 0) | (1UL << 1) | (1UL << 9) | (1UL << 10) | (1UL << 11) |          \
   (1UL << 12) | (1UL << 13) | (1UL << 10))
#endif
#define PWM_WO_OFFSET (6)
#endif

// Fallback for any MEGATINYCORE chip not explicitly matched above
#if defined(MEGATINYCORE) && !defined(ALL_GPIO)
#define ALL_GPIO 0x01FFFFUL  // Default to x16-like (17 GPIO)
#define ALL_ADC 0b11100001100111111
#define ALL_PWM ((1UL << 0) | (1UL << 1) | (1UL << 7) | (1UL << 8) | (1UL << 9))
#define PWM_WO_OFFSET (4)
#endif

#define INVALID_GPIO ((1UL << SDA) | (1UL << SCL) | \
                      ((uint32_t)CONFIG_UART_DEBUG << UART_DEBUG_RXD) | \
                      ((uint32_t)CONFIG_UART_DEBUG << UART_DEBUG_TXD)   |   \
                      ((uint32_t)CONFIG_INTERRUPT << CONFIG_INTERRUPT_PIN) | \
                      ((uint32_t)CONFIG_ADDR_0 << CONFIG_ADDR_0_PIN) | \
                      ((uint32_t)CONFIG_ADDR_1 << CONFIG_ADDR_1_PIN) | \
                      ((uint32_t)CONFIG_ADDR_2 << CONFIG_ADDR_2_PIN) | \
                      ((uint32_t)CONFIG_ADDR_3 << CONFIG_ADDR_3_PIN) | \
                      0)

#define VALID_GPIO (ALL_GPIO & ~INVALID_GPIO)
#define VALID_ADC (ALL_ADC & VALID_GPIO)
#define VALID_PWM (ALL_PWM & VALID_GPIO)

void Adafruit_seesawPeripheral_reset(void);
uint32_t Adafruit_seesawPeripheral_readBulk(uint32_t validpins);
void receiveEvent(int howMany);
void requestEvent(void);
void Adafruit_seesawPeripheral_run(void);
void Adafruit_seesawPeripheral_changedGPIO(void);

/****************************************************** global state */

#if CONFIG_FHT && defined(MEGATINYCORE)
volatile uint8_t i2c_buffer[3]; // Minimal I2C buffer w/FHT because RAM
volatile uint8_t fht_counter;   // For filling FHT input buffer
#else
volatile uint8_t i2c_buffer[32];
#endif

#if CONFIG_INTERRUPT
volatile uint32_t g_irqGPIO = 0;
volatile uint32_t g_irqFlags = 0;
volatile uint8_t IRQ_debounce_cntr = 0;
#define IRQ_DEBOUNCE_TICKS 3 // in millis
#endif

#if CONFIG_ADC
volatile uint8_t g_adcStatus = 0;
#endif
#if (CONFIG_PWM | CONFIG_PWM_16BIT)
volatile uint8_t g_pwmStatus = 0;
#endif
#if CONFIG_NEOPIXEL
volatile uint8_t g_neopixel_buf[CONFIG_NEOPIXEL_BUF_MAX];
volatile uint16_t g_neopixel_bufsize = 0;
volatile uint8_t g_neopixel_pin = 0;
#endif
#if CONFIG_UART
volatile uint8_t g_uart_buf[CONFIG_UART_BUF_MAX];
volatile uint8_t g_uart_status = 0;
volatile uint8_t g_uart_inten = 0;
volatile uint32_t g_uart_baud = 9600;
volatile uint8_t g_uart_tx_len = 0;
#endif

#if CONFIG_ENCODER

#define ENCODER_FLAG_FORW_EDGE1 0x01
#define ENCODER_FLAG_BACK_EDGE1 0x02
#define ENCODER_FLAG_FORW_EDGE2 0x04
#define ENCODER_FLAG_BACK_EDGE2 0x08
#define ENCODER_FLAG_MIDSTEP    0x10

#define BIT_IS_SET(x,b) (((x)&(1UL << b)) != 0)
#define BIT_IS_CLEAR(x,b) (((x)&(1UL << b)) == 0)

#define ENCODER0_INPUT_MASK ((1UL << CONFIG_ENCODER0_A_PIN) | (1UL << CONFIG_ENCODER0_B_PIN))

#ifdef CONFIG_ENCODER1_A_PIN
#define ENCODER1_INPUT_MASK ((1UL << CONFIG_ENCODER1_A_PIN) | (1UL << CONFIG_ENCODER1_B_PIN))
#else
#define ENCODER1_INPUT_MASK 0
#endif
#ifdef CONFIG_ENCODER2_A_PIN
#define ENCODER2_INPUT_MASK ((1UL << CONFIG_ENCODER2_A_PIN) | (1UL << CONFIG_ENCODER2_B_PIN))
#else
#define ENCODER2_INPUT_MASK 0
#endif
#ifdef CONFIG_ENCODER3_A_PIN
#define ENCODER3_INPUT_MASK ((1UL << CONFIG_ENCODER3_A_PIN) | (1UL << CONFIG_ENCODER3_B_PIN))
#else
#define ENCODER3_INPUT_MASK 0
#endif

#ifndef CONFIG_ENCODER_2TICKS
#define CONFIG_ENCODER_2TICKS 0

#endif

volatile int32_t g_enc_value[CONFIG_NUM_ENCODERS];
volatile int32_t g_enc_delta[CONFIG_NUM_ENCODERS];
volatile uint8_t g_enc_prev_pos[CONFIG_NUM_ENCODERS];
volatile uint8_t g_enc_flags[CONFIG_NUM_ENCODERS];

#endif

/****************************************************** Keypad support */

#if CONFIG_KEYPAD

#ifndef CONFIG_NUM_KEYPAD_KEYS
#define CONFIG_NUM_KEYPAD_KEYS 16  // Default to 16 keys (4x4 matrix)
#endif

#ifndef CONFIG_KEYPAD_FIFO_SIZE
#define CONFIG_KEYPAD_FIFO_SIZE 16  // Default FIFO size for events
#endif

// Edge detection flags (matches seesaw protocol)
#define KEYPAD_EDGE_HIGH    0  // Key is currently pressed
#define KEYPAD_EDGE_LOW     1  // Key is currently released
#define KEYPAD_EDGE_FALLING 2  // Key was just pressed (falling edge)
#define KEYPAD_EDGE_RISING  3  // Key was just released (rising edge)

// Per-key edge enable flags (bit positions in keyState.ACTIVE)
#define KEYPAD_EDGE_HIGH_EN    (1 << 1)
#define KEYPAD_EDGE_LOW_EN     (1 << 2)
#define KEYPAD_EDGE_FALLING_EN (1 << 3)
#define KEYPAD_EDGE_RISING_EN  (1 << 4)

// Keypad event structure (matches keyEventRaw in Adafruit_seesaw.h)
// Bits 0-1: EDGE type, Bits 2-7: key number (0-63)
typedef union {
  struct {
    uint8_t EDGE : 2;
    uint8_t NUM : 6;
  } bit;
  uint8_t reg;
} keypadEvent_t;

// Per-key state tracking
volatile uint8_t g_keypad_state[CONFIG_NUM_KEYPAD_KEYS];      // Current key states (1=pressed)
volatile uint8_t g_keypad_edge_config[CONFIG_NUM_KEYPAD_KEYS]; // Edge detection config per key

// Event FIFO
volatile keypadEvent_t g_keypad_fifo[CONFIG_KEYPAD_FIFO_SIZE];
volatile uint8_t g_keypad_fifo_head = 0;  // Write position
volatile uint8_t g_keypad_fifo_tail = 0;  // Read position
volatile uint8_t g_keypad_fifo_count = 0; // Number of events in FIFO

// Interrupt enable
volatile uint8_t g_keypad_inten = 0;

// Function prototypes
void Adafruit_seesawPeripheral_keypad_scan(void);
void Adafruit_seesawPeripheral_keypad_push_event(uint8_t key, uint8_t edge);

#endif // CONFIG_KEYPAD

/****************************************************** code */

// global address
uint8_t _i2c_addr = CONFIG_I2C_PERIPH_ADDR;

void Adafruit_seesawPeripheral_setDatecode(void) {

  char buf[12];
  char *bufp = buf;
  int month = 0, day = 0, year = 2000;
  static const char month_names[] = "JanFebMarAprMayJunJulAugSepOctNovDec";

  strncpy(buf, __DATE__, 11);
  buf[11] = 0;

  bufp[3] = 0;
  month = (strstr(month_names, bufp)-month_names)/3 + 1;

  bufp += 4;
  bufp[2] = 0;
  day = atoi(bufp);

  bufp += 3;
  year = atoi(bufp);

  DATE_CODE = day & 0x1F; // top 5 bits are day of month

  DATE_CODE <<= 4;
  DATE_CODE |= month & 0xF; // middle 4 bits are month

  DATE_CODE <<= 7;
  DATE_CODE |= (year - 2000) & 0x3F; // bottom 7 bits are year
}


void Adafruit_seesawPeripheral_setIRQ(void) {
  digitalWrite(CONFIG_INTERRUPT_PIN, LOW);
  pinMode(CONFIG_INTERRUPT_PIN, OUTPUT);
}

void Adafruit_seesawPeripheral_clearIRQ(void) {
  // time to turn off the IRQ pin?
  pinMode(CONFIG_INTERRUPT_PIN, INPUT_PULLUP); // open-drainish
}

bool Adafruit_seesawPeripheral_begin(void) {
  SEESAW_DEBUG(F("All GPIO: "));
  SEESAW_DEBUGLN(ALL_GPIO, HEX);
  SEESAW_DEBUG(F("Invalid: "));
  SEESAW_DEBUGLN(INVALID_GPIO, HEX);
  SEESAW_DEBUG(F("Valid: "));
  SEESAW_DEBUGLN(VALID_GPIO, HEX);

#ifdef CONFIG_INTERRUPT
  Adafruit_seesawPeripheral_clearIRQ();
#endif

  Adafruit_seesawPeripheral_reset();

  Wire.onReceive(receiveEvent);
  Wire.onRequest(requestEvent);
  return true;
}

void Adafruit_seesawPeripheral_reset(void) {
  Adafruit_seesawPeripheral_setDatecode();

  cli();

  SEESAW_DEBUGLN(F("Wire end"));
  Wire.end();

  // Not referenced after Wire.begin(), so this is now local
  uint8_t _i2c_addr = CONFIG_I2C_PERIPH_ADDR;

#if CONFIG_EEPROM
  _i2c_addr = EEPROM.read(EEPROM_I2C_ADDR);
  SEESAW_DEBUG(F("EEaddr: 0x"));
  SEESAW_DEBUGLN(_i2c_addr, HEX);
  if (_i2c_addr > 0x7F) {
    _i2c_addr = CONFIG_I2C_PERIPH_ADDR;
  }
#endif

#if CONFIG_ADDR_0
  pinMode(CONFIG_ADDR_0_PIN, INPUT_PULLUP);
  if (digitalRead(CONFIG_ADDR_0_PIN) == CONFIG_ADDR_INVERTED)
    _i2c_addr += 1;
#endif
#if CONFIG_ADDR_1
  pinMode(CONFIG_ADDR_1_PIN, INPUT_PULLUP);
  if (digitalRead(CONFIG_ADDR_1_PIN) == CONFIG_ADDR_INVERTED)
    _i2c_addr += 2;
#endif
#if CONFIG_ADDR_2
  pinMode(CONFIG_ADDR_2_PIN, INPUT_PULLUP);
  if (digitalRead(CONFIG_ADDR_2_PIN) == CONFIG_ADDR_INVERTED)
    _i2c_addr += 4;
#endif
#if CONFIG_ADDR_3
  pinMode(CONFIG_ADDR_3_PIN, INPUT_PULLUP);
  if (digitalRead(CONFIG_ADDR_3_PIN) == CONFIG_ADDR_INVERTED)
    _i2c_addr += 8;
#endif

  SEESAW_DEBUG(F("I2C 0x"));
  SEESAW_DEBUGLN(_i2c_addr, HEX);

  uint32_t pins = VALID_GPIO;
  for (uint8_t pin = 0; pin < 32; pin++) {
    if ((pins >> pin) & 0x1) {
      pinMode(pin, INPUT);
      digitalWrite(pin, 0);
#if USE_PINCHANGE_INTERRUPT
      detachInterrupt(digitalPinToInterrupt(pin));
#endif
    }
  }
#if CONFIG_INTERRUPT
  g_irqGPIO = 0;
  g_irqFlags = 0;
#endif
#if CONFIG_ADC
  g_adcStatus = 0;
#endif
#if CONFIG_PWM
  g_pwmStatus = 0;
  // PWM is provided by BSP's analogWrite() and tone()
#elif CONFIG_PWM_16BIT
  g_pwmStatus = 0;

#if defined(ATMEGA328_SERIES)
  // ATmega328P Timer1 16-bit PWM setup
  // Timer1 controls OC1A (D9) and OC1B (D10)
  TCCR1A = 0;  // Clear control register A
  TCCR1B = 0;  // Clear control register B
  TCNT1 = 0;   // Clear counter
  ICR1 = 0xFFFF;  // Set TOP to MAX for 16-bit resolution
  // Fast PWM mode 14 (WGM13:10 = 1110), prescaler = 1
  TCCR1A = _BV(WGM11);  // Fast PWM, TOP=ICR1
  TCCR1B = _BV(WGM13) | _BV(WGM12) | _BV(CS10);  // No prescaler
  // PWM outputs are enabled per-channel when duty cycle is set
#elif defined(MEGATINYCORE)
  // Original ATtiny TCA0 16-bit PWM setup
  // TCA0 is used for 16 bit PWM support
  takeOverTCA0();
  PORTMUX.CTRLC |= 0b111;    // Alternate WOx output pin locations
  TCA0.SINGLE.PER = 0xFFFF;  // Set TOP to MAX
  TCA0.SINGLE.CTRLB = 0x03;  // Single-slope PWM, WG outputs off
  TCA0.SINGLE.CTRLD = 0x00;  // Disable Split Mode
  TCA0.SINGLE.CTRLA = 0x01;  // Enable TCA0 peripheral
#endif

#endif
#if CONFIG_NEOPIXEL
  for (uint16_t i = 0; i < CONFIG_NEOPIXEL_BUF_MAX; i++) {
    g_neopixel_buf[i] = 0;
  }
  g_neopixel_bufsize = 0;
#endif
#if CONFIG_ENCODER
#if defined(CONFIG_ENCODER0_A_PIN)
  pinMode(CONFIG_ENCODER0_A_PIN, INPUT_PULLUP);
#endif
#if defined(CONFIG_ENCODER0_B_PIN)
  pinMode(CONFIG_ENCODER0_B_PIN, INPUT_PULLUP);
#endif
#if defined(CONFIG_ENCODER1_A_PIN)
  pinMode(CONFIG_ENCODER1_A_PIN, INPUT_PULLUP);
#endif
#if defined(CONFIG_ENCODER1_B_PIN)
  pinMode(CONFIG_ENCODER1_B_PIN, INPUT_PULLUP);
#endif
#if defined(CONFIG_ENCODER2_A_PIN)
  pinMode(CONFIG_ENCODER2_A_PIN, INPUT_PULLUP);
#endif
#if defined(CONFIG_ENCODER2_B_PIN)
  pinMode(CONFIG_ENCODER2_B_PIN, INPUT_PULLUP);
#endif
#if defined(CONFIG_ENCODER3_A_PIN)
  pinMode(CONFIG_ENCODER3_A_PIN, INPUT_PULLUP);
#endif
#if defined(CONFIG_ENCODER3_B_PIN)
  pinMode(CONFIG_ENCODER3_B_PIN, INPUT_PULLUP);
#endif

  for (uint8_t encodernum=0; encodernum<CONFIG_NUM_ENCODERS; encodernum++) {
    g_enc_value[encodernum] = 0;
    g_enc_delta[encodernum] = 0;
    g_enc_prev_pos[encodernum] = 0;
    g_enc_flags[encodernum] = 0;
  }
#endif

#if CONFIG_KEYPAD
  // Initialize keypad state
  for (uint8_t key = 0; key < CONFIG_NUM_KEYPAD_KEYS; key++) {
    g_keypad_state[key] = 0;
    g_keypad_edge_config[key] = 0;
  }
  // Clear FIFO
  g_keypad_fifo_head = 0;
  g_keypad_fifo_tail = 0;
  g_keypad_fifo_count = 0;
  g_keypad_inten = 0;

  // Set up keypad pins as inputs with pullups
  // User must define CONFIG_KEYPAD_PINS as an array of pin numbers
  #ifdef CONFIG_KEYPAD_PINS
  for (uint8_t i = 0; i < CONFIG_NUM_KEYPAD_KEYS; i++) {
    uint8_t pin = CONFIG_KEYPAD_PINS[i];
    if (pin < 32 && (VALID_GPIO & (1UL << pin))) {
      pinMode(pin, INPUT_PULLUP);
    }
  }
  #endif
#endif

#if CONFIG_FHT && defined(MEGATINYCORE)
#ifdef DISABLE_MILLIS
#if defined(MILLIS_USE_TIMERA0)
  TCA0.SPLIT.INTCTRL &= ~TCA_SPLIT_HUNF_bm;
#elif defined(MILLIS_USE_TIMERA1)
  TCA1.SPLIT.INTCTRL &= ~TCA_SPLIT_HUNF_bm;
#elif defined(MILLIS_USE_TIMERB0)
  TCB0.INTCTRL &= ~TCB_CAPT_bm;
#elif defined(MILLIS_USE_TIMERB1)
  TCB1.INTCTRL &= ~TCB_CAPT_bm;
#elif defined(MILLIS_USE_TIMERD0)
  TCD0.INTCTRL &= ~TCD_OVF_bm;
#endif
#endif // end DISABLE_MILLIS

  // ADC is configured for free-run mode with result-ready interrupt. 10-bit
  // w/4X accumulation for 12-bit result (0-4092, NOT 4095, because it's the
  // sum of four 10-bit values, not "true" 12-bit ADC reading).
  // Assuming 10 MHz or 20 MHz F_CPU, possible sampling rates are:
  // 1.25 MHz / 4X samples / 25 ADC cycles/sample -> 12500 Hz sample rate.
  // Highest frequency is 1/2 sampling rate, or 6250 Hz (just under G8 at
  // 6272 Hz). That’s a default that looks nice, but there's some adjustability
  // if needed, with the following top frequency range:
  // 1.25 MHz / 4X / (13+0)  = 24038 sample rate = 12019 peak freq
  // 1.25 MHz / 4X / (13+31) = 7102 sample rate = 3551 peak freq
  // Other F_CPU values (8, 12, 16) will result in different ranges.
  // Depending on what mic is used and its outpot voltage range, might want
  // to change AREF to another source. Right now it's the default VDD.

  fht_counter = 0; // For filling FHT input buffer

  ADC0.CTRLA = ADC_FREERUN_bm | ADC_ENABLE_bm; // 10-bit, free-run, enable ADC
  ADC0.CTRLB = ADC_SAMPNUM_ACC4_gc;   // Accumulate 4X (0-4092 (sic.) result)
  ADC0.CTRLC = ADC_SAMPCAP_bm |       // Reduced capacitance for >1V AREF
               ADC_REFSEL_VDDREF_gc | // VDD as AREF
#if F_CPU > 12000000
               ADC_PRESC_DIV16_gc; // 16:1 timer prescale (20->1.25 MHz)
#else
               ADC_PRESC_DIV8_gc; // 8:1 timer prescale (10->1.25 MHz)
#endif
  ADC0.CTRLD = 0; // No init or sample delay
  ADC0.MUXPOS = digitalPinToAnalogInput(FHT_DEFAULT_PIN);
  ADC0.SAMPCTRL = 12; // Add to usu. 13 ADC cycles for 25 cycles/sample
  ADC0.INTCTRL |= ADC_RESRDY_bm; // Enable result-ready interrupt
  ADC0.COMMAND |= ADC_STCONV_bm; // Start free-run conversion
#endif

#if CONFIG_UART
  CONFIG_UART_SERCOM.begin(g_uart_baud);
#endif

  Wire.begin(_i2c_addr);
  sei();
}

#if CONFIG_FHT && defined(MEGATINYCORE)
ISR(ADC0_RESRDY_vect) { // ADC conversion complete
  // Convert 12-bit ADC reading to signed value (+/-2K) and scale to 16-bit
  // space (scaling up isn't strictly required but FHT results look cleaner).
  // 2046 (not 2048) is intentional, see ADC notes above, don't "fix."
  fht_input[fht_counter] = (ADC0.RES - 2046) * 4;
  // Compare-before-increment allows a uint8_t counter, RAM's that tight.
  if (fht_counter == (FHT_N - 1)) { // FHT input buffer full?
    ADC0.INTCTRL &= ~ADC_RESRDY_bm; // Disable result-ready interrupt
  } else {
    fht_counter++;
  }
  // Interrupt flag is cleared automatically when reading ADC0.RES
}
#endif


uint32_t Adafruit_seesawPeripheral_readBulk(uint32_t validpins = VALID_GPIO) {
  uint32_t temp = 0;

#if defined(ATMEGA328_SERIES)
  // ATmega328P port read - uses PINB, PINC, PIND registers
  // Arduino pin mapping: D0-D7=PORTD, D8-D13=PORTB, A0-A5(D14-D19)=PORTC
  uint8_t pinb = PINB;
  uint8_t pinc = PINC;
  uint8_t pind = PIND;

  for (uint8_t pin = 0; pin < 20; pin++) {  // ATmega328 has 20 pins (0-19)
    temp >>= 1;
    if (validpins & 0x1) {
      uint8_t bit_val = 0;
      if (pin < 8) {
        // D0-D7 are on PORTD
        bit_val = (pind >> pin) & 0x1;
      } else if (pin < 14) {
        // D8-D13 are on PORTB (bits 0-5)
        bit_val = (pinb >> (pin - 8)) & 0x1;
      } else {
        // A0-A5 (D14-D19) are on PORTC (bits 0-5)
        bit_val = (pinc >> (pin - 14)) & 0x1;
      }
      if (bit_val) {
        temp |= 0x80000000UL;
      }
    }
    validpins >>= 1;
  }
  // Shift remaining bits for pins 20-31 (unused on ATmega328)
  temp >>= 12;

#elif defined(MEGATINYCORE)
  // Original ATtiny code using VPORT registers
  // read all ports
  uint8_t port_reads[3] = {0, 0, 0};
  port_reads[0] = VPORTA.IN;
  port_reads[1] = VPORTB.IN;
  port_reads[2] = VPORTC.IN;

  //pinMode(1, OUTPUT);
  //digitalWriteFast(1, HIGH);
  for (uint8_t pin = 0; pin < 32; pin++) {
    temp >>= 1;
    if (validpins & 0x1) {
      uint8_t mask = 1 << digital_pin_to_bit_position[pin];
      uint8_t port = digital_pin_to_port[pin];
      if (port_reads[port] & mask) {
        temp |= 0x80000000UL;
      }
    }
    validpins >>= 1;
  }
  //digitalWriteFast(1, LOW);

#else
  // Fallback for other AVR - use digitalRead (slower but portable)
  for (uint8_t pin = 0; pin < 32; pin++) {
    temp >>= 1;
    if (validpins & 0x1) {
      if (digitalRead(pin)) {
        temp |= 0x80000000UL;
      }
    }
    validpins >>= 1;
  }
#endif

  return temp;
}

void Adafruit_seesawPeripheral_write32(uint32_t value) {
  Wire.write(value >> 24);
  Wire.write(value >> 16);
  Wire.write(value >> 8);
  Wire.write(value);
  return;
}

/****************************************************** Keypad functions */

#if CONFIG_KEYPAD

// Push an event onto the keypad FIFO
void Adafruit_seesawPeripheral_keypad_push_event(uint8_t key, uint8_t edge) {
  if (g_keypad_fifo_count < CONFIG_KEYPAD_FIFO_SIZE) {
    keypadEvent_t evt;
    evt.bit.NUM = key & 0x3F;  // 6 bits for key number
    evt.bit.EDGE = edge & 0x03; // 2 bits for edge type
    g_keypad_fifo[g_keypad_fifo_head] = evt;
    g_keypad_fifo_head = (g_keypad_fifo_head + 1) % CONFIG_KEYPAD_FIFO_SIZE;
    g_keypad_fifo_count++;

    SEESAW_DEBUG(F("Key "));
    SEESAW_DEBUG(key);
    SEESAW_DEBUG(F(" edge "));
    SEESAW_DEBUGLN(edge);

    // Trigger interrupt if enabled
    #if CONFIG_INTERRUPT
    if (g_keypad_inten) {
      Adafruit_seesawPeripheral_setIRQ();
    }
    #endif
  }
}

// Scan keypad pins and generate events
// Call this from the run() function periodically
void Adafruit_seesawPeripheral_keypad_scan(void) {
  #ifdef CONFIG_KEYPAD_PINS
  for (uint8_t i = 0; i < CONFIG_NUM_KEYPAD_KEYS; i++) {
    uint8_t pin = CONFIG_KEYPAD_PINS[i];
    if (pin >= 32 || !(VALID_GPIO & (1UL << pin))) continue;

    // Read current state (inverted because of pullup - LOW = pressed)
    uint8_t current = digitalRead(pin) ? 0 : 1;
    uint8_t previous = g_keypad_state[i];
    uint8_t config = g_keypad_edge_config[i];

    // Check for state changes and generate events
    if (current != previous) {
      // State changed
      if (current && (config & KEYPAD_EDGE_FALLING_EN)) {
        // Key pressed (falling edge on pin, rising edge in logic)
        Adafruit_seesawPeripheral_keypad_push_event(i, KEYPAD_EDGE_FALLING);
      }
      if (!current && (config & KEYPAD_EDGE_RISING_EN)) {
        // Key released (rising edge on pin, falling edge in logic)
        Adafruit_seesawPeripheral_keypad_push_event(i, KEYPAD_EDGE_RISING);
      }
      g_keypad_state[i] = current;
    }

    // Also check for level events (continuous while in state)
    if (current && (config & KEYPAD_EDGE_HIGH_EN)) {
      Adafruit_seesawPeripheral_keypad_push_event(i, KEYPAD_EDGE_HIGH);
    }
    if (!current && (config & KEYPAD_EDGE_LOW_EN)) {
      Adafruit_seesawPeripheral_keypad_push_event(i, KEYPAD_EDGE_LOW);
    }
  }
  #endif
}

#endif // CONFIG_KEYPAD

#include "Adafruit_seesawPeripheral_main.h"
#include "Adafruit_seesawPeripheral_receive.h"
#include "Adafruit_seesawPeripheral_request.h"
#endif
