# Adafruit seesawPeripheral Library[![Build Status](https://github.com/adafruit/Adafruit_seesawPeripheral/workflows/Arduino%20Library%20CI/badge.svg)](https://github.com/adafruit/Adafruit_seesawPeripheral/actions)[![Documentation](https://github.com/adafruit/ci-arduino/blob/master/assets/doxygen_badge.svg)](http://adafruit.github.io/Adafruit_seesawPeripheral/html/index.html)

Library for making seesaw i2c peripherals in Arduino core

To install, use the Arduino Library Manager and search for 'Adafruit seesaw Peripheral' and install the library.

Compile with something like this, but check the example for UART debug flags and other comment sections:
```
arduino-cli compile --fqbn arduino:avr:nano --build-property "build.extra_flags=-DCONFIG_CLOCK_8MHZ=1" --build-property "build.f_cpu=8000000L" --library ./ --upload --port /dev/ttyUSB0 ./examples/example_atmega328_keypad_matrix/example_atmega328_keypad_matrix.ino
```
(Running an Arduino Uno / Nano / atmega328p at 10MHz or less lowers the minimum voltage to 2.7volts, so it can be powered by a StemmaQT 3.3v power line into the 5V pin)