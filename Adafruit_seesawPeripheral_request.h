// Hardware ID definitions for supported chips
// ATmega328 series (Arduino Uno, Nano, Pro Mini, etc.)
#if defined(__AVR_ATmega328P__)
#define SEESAW_HW_ID 0x87  // ATmega328P
#elif defined(__AVR_ATmega328PB__)
#define SEESAW_HW_ID 0x8A  // ATmega328PB (extended variant)
#elif defined(__AVR_ATmega328__)
#define SEESAW_HW_ID 0x87  // ATmega328 (non-P variant)
// ATtiny x06 series
#elif defined(ARDUINO_AVR_ATtiny406) || defined(ARDUINO_AVR_ATtiny806) || \
      defined(ARDUINO_AVR_ATtiny1606) || defined(ARDUINO_AVR_ATtiny3206)
#define SEESAW_HW_ID 0x84
// ATtiny x07 series
#elif defined(ARDUINO_AVR_ATtiny407) || defined(ARDUINO_AVR_ATtiny807) || \
      defined(ARDUINO_AVR_ATtiny1607) || defined(ARDUINO_AVR_ATtiny3207)
#define SEESAW_HW_ID 0x85
// ATtiny x16 series
#elif defined(ARDUINO_AVR_ATtiny416) || defined(ARDUINO_AVR_ATtiny816) || \
      defined(ARDUINO_AVR_ATtiny1616) || defined(ARDUINO_AVR_ATtiny3216)
#define SEESAW_HW_ID 0x86
// ATtiny x17 series
#elif defined(ARDUINO_AVR_ATtiny417) || defined(ARDUINO_AVR_ATtiny817) || \
      defined(ARDUINO_AVR_ATtiny1617) || defined(ARDUINO_AVR_ATtiny3217)
#define SEESAW_HW_ID 0x87
// ATtiny x26 series (newer)
#elif defined(ARDUINO_AVR_ATtiny426) || defined(ARDUINO_AVR_ATtiny826) || \
      defined(ARDUINO_AVR_ATtiny1626) || defined(ARDUINO_AVR_ATtiny3226)
#define SEESAW_HW_ID 0x88
// ATtiny x27 series (newer)
#elif defined(ARDUINO_AVR_ATtiny427) || defined(ARDUINO_AVR_ATtiny827) || \
      defined(ARDUINO_AVR_ATtiny1627) || defined(ARDUINO_AVR_ATtiny3227)
#define SEESAW_HW_ID 0x89
// Fallback for megaTinyCore with generic board settings
#elif defined(MEGATINYCORE)
#define SEESAW_HW_ID 0x86  // Default to x16 series ID
#else
#error "Unsupported chip variant selected"
#endif

extern volatile uint32_t g_bufferedBulkGPIORead;

/***************************** data read */
void requestEvent(void) {
  // SEESAW_DEBUGLN(F("Requesting data"));
  uint8_t base_cmd = i2c_buffer[0];
  uint8_t module_cmd = i2c_buffer[1];

  if (base_cmd == SEESAW_STATUS_BASE) {
    if (module_cmd == SEESAW_STATUS_HW_ID) {
      Wire.write(SEESAW_HW_ID); // instant reply
    }
    else if (module_cmd == SEESAW_STATUS_VERSION) {
      Adafruit_seesawPeripheral_write32(CONFIG_VERSION | DATE_CODE); // instant reply
    }
    else if (module_cmd == SEESAW_STATUS_OPTIONS) {
      // Return a 32-bit options word indicating which modules are available
      // Each bit position corresponds to a module base address
      uint32_t options = 0;
      options |= (1UL << SEESAW_STATUS_BASE);     // Status module always available
      options |= (1UL << SEESAW_GPIO_BASE);       // GPIO always available
#if CONFIG_UART
      options |= (1UL << SEESAW_SERCOM0_BASE);    // UART/SERCOM
#endif
#if CONFIG_PWM || CONFIG_PWM_16BIT
      options |= (1UL << SEESAW_TIMER_BASE);      // Timer/PWM
#endif
#if CONFIG_ADC
      options |= (1UL << SEESAW_ADC_BASE);        // ADC
#endif
#if CONFIG_INTERRUPT
      options |= (1UL << SEESAW_INTERRUPT_BASE);  // Interrupt
#endif
#if CONFIG_EEPROM
      options |= (1UL << SEESAW_EEPROM_BASE);     // EEPROM
#endif
#if CONFIG_NEOPIXEL
      options |= (1UL << SEESAW_NEOPIXEL_BASE);   // NeoPixel
#endif
#if CONFIG_KEYPAD
      options |= (1UL << SEESAW_KEYPAD_BASE);     // Keypad
#endif
#if CONFIG_ENCODER
      options |= (1UL << SEESAW_ENCODER_BASE);    // Encoder
#endif
      Adafruit_seesawPeripheral_write32(options);
    }
    else if (module_cmd == SEESAW_STATUS_TEMP) {
      // Return internal temperature sensor reading
      // Format: 32-bit value, multiply by 0.00001525878 to get Celsius
#if defined(ATMEGA328_SERIES)
      // ATmega328P has internal temperature sensor on ADC channel 8
      // Save current ADMUX and ADCSRA settings
      uint8_t old_admux = ADMUX;
      uint8_t old_adcsra = ADCSRA;

      // Configure ADC for internal temperature sensor
      // REFS1:0 = 11 (internal 1.1V reference)
      // MUX3:0 = 1000 (temperature sensor)
      ADMUX = _BV(REFS1) | _BV(REFS0) | 0x08;
      ADCSRA = _BV(ADEN) | _BV(ADPS2) | _BV(ADPS1) | _BV(ADPS0); // Enable ADC, prescaler 128

      // Discard first reading after reference change
      ADCSRA |= _BV(ADSC);
      while (ADCSRA & _BV(ADSC));

      // Take actual reading
      ADCSRA |= _BV(ADSC);
      while (ADCSRA & _BV(ADSC));
      uint16_t adc_val = ADC;

      // Restore ADC settings
      ADMUX = old_admux;
      ADCSRA = old_adcsra;

      // Convert to format expected by CircuitPython driver
      // Driver does: 0.00001525878 * ret to get Celsius
      // ATmega328P formula: T = (ADC - 324.31) / 1.22
      // We need to return value where val * 0.00001525878 = T
      // So val = T / 0.00001525878 = T * 65536
      // Since T = (ADC - 324.31) / 1.22, we compute accordingly
      int32_t temp_c_x100 = ((int32_t)adc_val - 324) * 100 / 122;  // Temp in Celsius * 100
      uint32_t temp_val = (uint32_t)(temp_c_x100 * 655);  // Scale for driver formula

      Adafruit_seesawPeripheral_write32(temp_val);
#elif defined(MEGATINYCORE)
      // ATtiny series temperature reading
      // Use internal temperature sensor via ADC
      // Note: 0/1-series and 2-series have different ADC register layouts
      uint16_t adc_val = 0;

#if defined(__AVR_TINY_2__)  // 2-series ATtiny (x26/x27)
      // 2-series uses different ADC peripheral
      uint8_t old_muxpos = ADC0.MUXPOS;
      ADC0.MUXPOS = ADC_MUXPOS_TEMPSENSE_gc;

      // Single conversion on 2-series
      ADC0.CTRLA |= ADC_ENABLE_bm;
      ADC0.COMMAND = ADC_START_IMMEDIATE_gc;
      while (!(ADC0.INTFLAGS & ADC_SAMPRDY_bm));
      adc_val = ADC0.RESULT;
      ADC0.INTFLAGS = ADC_SAMPRDY_bm;  // Clear flag

      ADC0.MUXPOS = old_muxpos;  // Restore
#else
      // 0/1-series ATtiny
      uint8_t old_muxpos = ADC0.MUXPOS;
      ADC0.MUXPOS = ADC_MUXPOS_TEMPSENSE_gc;

      ADC0.COMMAND = ADC_STCONV_bm;
      while (!(ADC0.INTFLAGS & ADC_RESRDY_bm));
      adc_val = ADC0.RES;
      ADC0.INTFLAGS = ADC_RESRDY_bm;  // Clear flag

      ADC0.MUXPOS = old_muxpos;  // Restore
#endif

      // ATtiny temp calculation depends on factory calibration values
      // For simplicity, return raw ADC scaled for driver
      uint32_t temp_val = (uint32_t)adc_val << 16;
      Adafruit_seesawPeripheral_write32(temp_val);
#else
      // Unsupported platform - return 0
      Adafruit_seesawPeripheral_write32(0);
#endif
    }
  } else if (base_cmd == SEESAW_GPIO_BASE) {
    if (module_cmd == SEESAW_GPIO_BULK) {
      Adafruit_seesawPeripheral_write32(g_bufferedBulkGPIORead); // instant reply because we did the write before
#if CONFIG_INTERRUPT
      g_irqFlags = 0; // reading the gpio pins clears them
      Adafruit_seesawPeripheral_clearIRQ();
#endif
    }
#if CONFIG_INTERRUPT
    else if (module_cmd == SEESAW_GPIO_INTFLAG) {
      Adafruit_seesawPeripheral_write32(g_irqFlags);
      g_irqFlags = 0; // reading the flags clears them
      Adafruit_seesawPeripheral_clearIRQ();
    }
#endif
  }

#if CONFIG_ADC
  else if (base_cmd == SEESAW_ADC_BASE) {
    if (module_cmd >= SEESAW_ADC_CHANNEL_OFFSET) {
      Wire.write(g_bufferedADCRead >> 8);
      Wire.write(g_bufferedADCRead);
    } else if (module_cmd == SEESAW_ADC_STATUS) {
      Wire.write(g_adcStatus);
    }
  }
#endif

#if CONFIG_EEPROM
  else if (base_cmd == SEESAW_EEPROM_BASE) {
    Wire.write(EEPROM.read(module_cmd % EEPROM.length()));
  }
#endif

#if CONFIG_ENCODER
  else if (base_cmd == SEESAW_ENCODER_BASE) {
    uint8_t encoder_num = 0;
    if ((module_cmd & 0xF0) == SEESAW_ENCODER_POSITION) {
      encoder_num = module_cmd & 0x0F;
      if (encoder_num < CONFIG_NUM_ENCODERS){
        Adafruit_seesawPeripheral_write32(g_enc_value[encoder_num]);
        g_enc_delta[encoder_num] = 0;
      }
    }
    else if ((module_cmd & 0xF0) == SEESAW_ENCODER_DELTA) {
      encoder_num = module_cmd & 0x0F;
      if (encoder_num < CONFIG_NUM_ENCODERS){
        Adafruit_seesawPeripheral_write32(g_enc_delta[encoder_num]);
        g_enc_delta[encoder_num] = 0;
      }
    }
  }
#endif

#if CONFIG_FHT && defined(MEGATINYCORE)
  else if (base_cmd == SEESAW_SPECTRUM_BASE) {
    // TO DO: change to A/B/C/D results if we decide on FHT_N = 256.
    // That will require changes in Adafruit_Seesaw as well. Note that
    // this will only be possible if using an ATtiny part with 1K RAM
    // or better; won't fit on smaller devices.
    if (module_cmd == SEESAW_SPECTRUM_RESULTS_LOWER) {
      Wire.write(fht_log_out, 32);
    } else if (module_cmd == SEESAW_SPECTRUM_RESULTS_UPPER) {
      Wire.write(&fht_log_out[32], 32);
    } else if (module_cmd == SEESAW_SPECTRUM_CHANNEL) {
      // TO DO: this should re-map the current MUXPOS setting to the same
      // channel mapping as is used in Adafruit_seesawPeripheral_receive.h --
      // see notes over there. Prob just two values. For now though, for the
      // sake of initial testing, this just returns the raw MUXPOS setting.
      Wire.write(ADC0.MUXPOS); // Return current ADC channel
    } else if (module_cmd == SEESAW_SPECTRUM_RATE) {
      Wire.write(ADC0.SAMPCTRL); // Return current sample rate index
    }
  }
#endif

#if CONFIG_UART
  else if (base_cmd == SEESAW_SERCOM0_BASE) {
    if (module_cmd == SEESAW_SERCOM_STATUS) {
      Wire.write(g_uart_status);
    } else if (module_cmd == SEESAW_SERCOM_INTEN) {
      Wire.write(g_uart_inten);
    } else if (module_cmd == SEESAW_SERCOM_BAUD) {
      Wire.write((g_uart_baud >> 24) & 0xFF);
      Wire.write((g_uart_baud >> 16) & 0xFF);
      Wire.write((g_uart_baud >> 8) & 0xFF);
      Wire.write(g_uart_baud & 0xFF);
    } else if (module_cmd == SEESAW_SERCOM_DATA) {
      Wire.write(CONFIG_UART_SERCOM.read());
    }
  }
#endif

#if CONFIG_KEYPAD
  else if (base_cmd == SEESAW_KEYPAD_BASE) {
    if (module_cmd == SEESAW_KEYPAD_STATUS) {
      // Return status byte (bit 0 = events available)
      Wire.write(g_keypad_fifo_count > 0 ? 0x01 : 0x00);
    }
    else if (module_cmd == SEESAW_KEYPAD_COUNT) {
      // Return number of events in FIFO
      Wire.write(g_keypad_fifo_count);
    }
    else if (module_cmd == SEESAW_KEYPAD_FIFO) {
      // Return all events from FIFO
      // The host reads this after checking COUNT
      uint8_t count = g_keypad_fifo_count;
      for (uint8_t i = 0; i < count && g_keypad_fifo_count > 0; i++) {
        Wire.write(g_keypad_fifo[g_keypad_fifo_tail].reg);
        g_keypad_fifo_tail = (g_keypad_fifo_tail + 1) % CONFIG_KEYPAD_FIFO_SIZE;
        g_keypad_fifo_count--;
      }
      // Clear interrupt if FIFO is now empty
      #if CONFIG_INTERRUPT
      if (g_keypad_fifo_count == 0) {
        Adafruit_seesawPeripheral_clearIRQ();
      }
      #endif
    }
  }
#endif

  else {
    SEESAW_DEBUG(F("Unhandled cmd 0x"));
    SEESAW_DEBUGLN(base_cmd, HEX);
  }
}
