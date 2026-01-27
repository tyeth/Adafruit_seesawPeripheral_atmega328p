// Seesaw peripheral for M5Stack FACES Keyboard Panel
// Makes the FACES keyboard work as a seesaw keypad device
// Compatible with Adafruit_seesaw keypad module in CircuitPython/Arduino
//
// Hardware: M5Stack FACES Keyboard (ATmega328P based)
// Original firmware: https://github.com/m5stack/FACES-Firmware
//
// Pin mapping (directly from M5Stack FACES hardware):
//   Columns: PC1, PC2, PC3 (Arduino A1, A2, A3)
//   Rows: PD0-PD7 (Arduino D0-D7), PB0-PB1 (Arduino D8-D9)
//   Special keys: PB3=Fn(D11), PB4=Alt(D12), PB5=aA/Shift(D13), PB6=Enter, PB7=Sym
//   IRQ: PB2 (Arduino D10)
//   I2C: PC4=SDA (A4), PC5=SCL (A5)

#define PRODUCT_CODE            0x4D35  // "M5" for M5Stack
#define CONFIG_I2C_PERIPH_ADDR  0x5E    // Different from original 0x08 to allow coexistence

// Note: Original M5Stack uses 0x08, but that conflicts with some devices
// Change to 0x08 if you need original address compatibility
// #define CONFIG_I2C_PERIPH_ADDR  0x08

// IRQ on PB2 (Arduino D10)
#define CONFIG_INTERRUPT_PIN    10  // PB2

// Enable keypad support - we handle scanning ourselves for FACES matrix
#define CONFIG_KEYPAD           1
#define CONFIG_NUM_KEYPAD_KEYS  40  // 35 matrix keys + 5 modifier keys
#define CONFIG_KEYPAD_FIFO_SIZE 16

// We don't use CONFIG_KEYPAD_PINS - instead we scan the FACES matrix directly

#include "Adafruit_seesawPeripheral.h"

// Key indices (match row/column scan order)
// Matrix keys 0-34, then modifiers 35-39
#define KEY_Q       0
#define KEY_W       1
#define KEY_E       2
#define KEY_R       3
#define KEY_T       4
#define KEY_Y       5
#define KEY_U       6
#define KEY_I       7
#define KEY_O       8
#define KEY_P       9
#define KEY_A       10
#define KEY_S       11
#define KEY_D       12
#define KEY_F       13
#define KEY_G       14
#define KEY_H       15
#define KEY_J       16
#define KEY_K       17
#define KEY_L       18
#define KEY_DEL     19
#define KEY_ALT     20  // Also as modifier at 36
#define KEY_Z       21
#define KEY_X       22
#define KEY_C       23
#define KEY_V       24
#define KEY_B       25
#define KEY_N       26
#define KEY_M       27
#define KEY_DOLLAR  28
#define KEY_ENTER   29  // Also as modifier at 37
#define KEY_SHIFT   30  // aA key, also as modifier at 35
#define KEY_0       31
#define KEY_SPACE   32
#define KEY_SYM     33  // Also as modifier at 38
#define KEY_FN      34  // Also as modifier at 39

// Modifier keys (reported separately for tracking state)
#define KEY_MOD_SHIFT  35
#define KEY_MOD_ALT    36
#define KEY_MOD_ENTER  37
#define KEY_MOD_SYM    38
#define KEY_MOD_FN     39

// ASCII key map: [key_index][mode] - for reference, matches original M5Stack KeyMap
// Modes: 0=lowercase, 1=uppercase, 2=SYM, 3=FN, 4=ALT
// NOTE: This array is not used by seesaw protocol (which reports raw key indices).
//       Uncomment if you want to add ASCII conversion in firmware.
/*
const uint8_t KEY_ASCII_MAP[35][5] PROGMEM = {
  {'q', 'Q', '#', '~', 0},    // 0: Q
  {'w', 'W', '1', '^', 0},    // 1: W
  {'e', 'E', '2', '&', 0},    // 2: E
  {'r', 'R', '3', '`', 0},    // 3: R
  {'t', 'T', '(', '<', 0},    // 4: T
  {'y', 'Y', ')', '>', 0},    // 5: Y
  {'u', 'U', '_', '{', 0},    // 6: U
  {'i', 'I', '-', '}', 0},    // 7: I
  {'o', 'O', '+', '[', 0},    // 8: O
  {'p', 'P', '@', ']', 0},    // 9: P
  {'a', 'A', '*', '|', 0},    // 10: A
  {'s', 'S', '4', '=', 0},    // 11: S
  {'d', 'D', '5', '\\', 0},   // 12: D
  {'f', 'F', '6', '%', 0},    // 13: F
  {'g', 'G', '/', 0, 0},      // 14: G
  {'h', 'H', ':', 0, 0},      // 15: H
  {'j', 'J', ';', 0, 0},      // 16: J
  {'k', 'K', '\'', 0, 0},     // 17: K
  {'l', 'L', '"', 0, 0},      // 18: L
  {0x08, 0x08, 0x7F, 0x08, 0},// 19: DEL/Backspace
  {0, 0, 0, 0, 0},            // 20: ALT (modifier)
  {'z', 'Z', '7', 0, 0},      // 21: Z
  {'x', 'X', '8', 0, 0},      // 22: X
  {'c', 'C', '9', 0, 0},      // 23: C
  {'v', 'V', '?', 0, 0},      // 24: V
  {'b', 'B', '!', 0, 0},      // 25: B
  {'n', 'N', ',', 0, 0},      // 26: N
  {'m', 'M', '.', 0, 0},      // 27: M
  {'$', '$', 0, 0, 0},        // 28: $
  {0x0D, 0x0D, 0x0D, 0x0D, 0},// 29: Enter
  {0, 0, 0, 0, 0},            // 30: aA/Shift (modifier)
  {'0', '0', '0', '0', 0},    // 31: 0
  {' ', ' ', ' ', ' ', 0},    // 32: Space
  {0, 0, 0, 0, 0},            // 33: SYM (modifier)
  {0, 0, 0x1B, 0, 0},         // 34: FN (ESC in SYM mode)
};
*/

// Previous key states for edge detection
static uint8_t prevKeyState[CONFIG_NUM_KEYPAD_KEYS];

// Scan the FACES keyboard matrix
// This replaces the default keypad scan since FACES uses a different matrix layout
void scanFacesKeyboard() {
  uint8_t keyState[CONFIG_NUM_KEYPAD_KEYS] = {0};

  // Read modifier keys directly from PORTB
  uint8_t pinb = PINB;
  keyState[KEY_MOD_FN]    = !(pinb & 0x08);  // PB3
  keyState[KEY_MOD_ALT]   = !(pinb & 0x10);  // PB4
  keyState[KEY_MOD_SHIFT] = !(pinb & 0x20);  // PB5
  keyState[KEY_MOD_ENTER] = !(pinb & 0x40);  // PB6
  keyState[KEY_MOD_SYM]   = !(pinb & 0x80);  // PB7

  // Also map to matrix positions for compatibility
  keyState[KEY_FN]    = keyState[KEY_MOD_FN];
  keyState[KEY_ALT]   = keyState[KEY_MOD_ALT];
  keyState[KEY_SHIFT] = keyState[KEY_MOD_SHIFT];
  keyState[KEY_ENTER] = keyState[KEY_MOD_ENTER];
  keyState[KEY_SYM]   = keyState[KEY_MOD_SYM];

  // Scan matrix: 3 column groups, each reads different rows
  // Column select via PC1-PC3

  // Column group 1: PC2=0, PC3=1, PC1=1 -> Keys Q,W,E,R,T,Y,U,I,O,P (rows via PIND/PINB)
  PORTC = (PORTC & 0xF1) | 0x0A;  // PC1=1, PC2=0, PC3=1
  delayMicroseconds(10);
  uint8_t pind = PIND;
  pinb = PINB;

  keyState[KEY_Q] = !(pinb & 0x02);  // R1 = PB1
  keyState[KEY_W] = !(pinb & 0x01);  // R2 = PB0
  keyState[KEY_E] = !(pind & 0x20);  // R3 = PD5
  keyState[KEY_R] = !(pind & 0x40);  // R4 = PD6
  keyState[KEY_T] = !(pind & 0x80);  // R5 = PD7
  keyState[KEY_Y] = !(pind & 0x10);  // R6 = PD4
  keyState[KEY_U] = !(pind & 0x08);  // R7 = PD3
  keyState[KEY_I] = !(pind & 0x04);  // R8 = PD2
  keyState[KEY_O] = !(pind & 0x02);  // R9 = PD1
  keyState[KEY_P] = !(pind & 0x01);  // R10 = PD0

  // Column group 2: PC2=1, PC3=0, PC1=1 -> Keys A,S,D,F,G,H,J,K,L,DEL
  PORTC = (PORTC & 0xF1) | 0x06;  // PC1=1, PC2=1, PC3=0
  delayMicroseconds(10);
  pind = PIND;
  pinb = PINB;

  keyState[KEY_A]   = !(pinb & 0x02);  // R1 = PB1
  keyState[KEY_S]   = !(pinb & 0x01);  // R2 = PB0
  keyState[KEY_D]   = !(pind & 0x20);  // R3 = PD5
  keyState[KEY_F]   = !(pind & 0x40);  // R4 = PD6
  keyState[KEY_G]   = !(pind & 0x80);  // R5 = PD7
  keyState[KEY_H]   = !(pind & 0x10);  // R6 = PD4
  keyState[KEY_J]   = !(pind & 0x08);  // R7 = PD3
  keyState[KEY_K]   = !(pind & 0x04);  // R8 = PD2
  keyState[KEY_L]   = !(pind & 0x02);  // R9 = PD1
  keyState[KEY_DEL] = !(pind & 0x01);  // R10 = PD0

  // Column group 3: PC2=1, PC3=1, PC1=0 -> Keys 0,Z,X,C,V,B,N,M,$,Space
  // Note: This column has different row mapping than cols 1&2!
  // Original M5Stack returns: PB1->31(0), PB0->21(Z), PD5->22(X), PD6->23(C),
  //   PD7->24(V), PD4->25(B), PD3->26(N), PD2->27(M), PD1->28($), PD0->32(SPACE)
  PORTC = (PORTC & 0xF1) | 0x0C;  // PC1=0, PC2=1, PC3=1
  delayMicroseconds(10);
  pind = PIND;
  pinb = PINB;

  keyState[KEY_0]      = !(pinb & 0x02);  // PB1 -> key 31 ('0')
  keyState[KEY_Z]      = !(pinb & 0x01);  // PB0 -> key 21 (Z)
  keyState[KEY_X]      = !(pind & 0x20);  // PD5 -> key 22 (X)
  keyState[KEY_C]      = !(pind & 0x40);  // PD6 -> key 23 (C)
  keyState[KEY_V]      = !(pind & 0x80);  // PD7 -> key 24 (V)
  keyState[KEY_B]      = !(pind & 0x10);  // PD4 -> key 25 (B)
  keyState[KEY_N]      = !(pind & 0x08);  // PD3 -> key 26 (N)
  keyState[KEY_M]      = !(pind & 0x04);  // PD2 -> key 27 (M)
  keyState[KEY_DOLLAR] = !(pind & 0x02);  // PD1 -> key 28 ($)
  keyState[KEY_SPACE]  = !(pind & 0x01);  // PD0 -> key 32 (SPACE)

  // Reset column lines
  PORTC = (PORTC & 0xF1) | 0x0E;  // All high (inactive)

  // Compare with previous state and generate events
  for (uint8_t key = 0; key < CONFIG_NUM_KEYPAD_KEYS; key++) {
    uint8_t current = keyState[key];
    uint8_t previous = prevKeyState[key];
    uint8_t config = g_keypad_edge_config[key];

    if (current != previous) {
      // State changed
      if (current && (config & KEYPAD_EDGE_FALLING_EN)) {
        Adafruit_seesawPeripheral_keypad_push_event(key, KEYPAD_EDGE_FALLING);
      }
      if (!current && (config & KEYPAD_EDGE_RISING_EN)) {
        Adafruit_seesawPeripheral_keypad_push_event(key, KEYPAD_EDGE_RISING);
      }
    }

    // Level events
    if (current && (config & KEYPAD_EDGE_HIGH_EN)) {
      Adafruit_seesawPeripheral_keypad_push_event(key, KEYPAD_EDGE_HIGH);
    }
    if (!current && (config & KEYPAD_EDGE_LOW_EN)) {
      Adafruit_seesawPeripheral_keypad_push_event(key, KEYPAD_EDGE_LOW);
    }

    prevKeyState[key] = current;
  }
}

void setup() {
  // Configure FACES keyboard pins
  // PORTB: PB0,PB1 = row inputs, PB2 = IRQ output, PB3-7 = modifier inputs
  DDRB = 0x04;    // PB2 output, rest inputs
  PORTB = 0xFB;   // Pull-ups on inputs, IRQ high (inactive)

  // PORTC: PC1-3 = column outputs, PC4-5 = I2C (handled by Wire)
  DDRC |= 0x0E;   // PC1-3 outputs
  PORTC |= 0x0E;  // All columns high (inactive)

  // PORTD: All row inputs with pull-ups
  DDRD = 0x00;
  PORTD = 0xFF;

  // Initialize previous key states
  for (uint8_t i = 0; i < CONFIG_NUM_KEYPAD_KEYS; i++) {
    prevKeyState[i] = 0;
  }

  Adafruit_seesawPeripheral_begin();
}

void loop() {
  // Scan keyboard every 5ms
  static uint32_t lastScan = 0;
  uint32_t now = millis();
  if ((now - lastScan) >= 5) {
    scanFacesKeyboard();
    lastScan = now;
  }

  // Run seesaw peripheral (handles I2C, other features)
  Adafruit_seesawPeripheral_run();
}
