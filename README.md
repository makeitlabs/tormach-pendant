## Emulated Tormach PathPilot Operator Console Controls

This is the 2023 version of the custom pendant built for the MakeIt Labs Tormach CNC mill.  Unlike the legacy version (circa 2017, contained in branch pathpilot1-custom-integration), which required extensive customization to PathPilot 1.x to function, this version emulates the official Tormach controls that have recently become available.  This means it works with unmodified versions of PathPilot 2.9.2 onward - a vast improvement, allowing us to keep the control software up-to-date without a tedious manual patching process each time.

### Physical pendant based on Teensy LC

  - physical CYCLE START, FEED HOLD, STOP buttons (potentially others)
  - physical rotary encoder for max velocity control 0-100%, good when proving new G-Code or for new users
  - physical rotary encoder for feed rate override 0-200%
  - physical rotary encoder for spindle RPM override 0-200%
  - RGB LED status indicator for error conditions, ready, etc.
  - handheld pendant wheel support for step/continuous jogging of X,Y,Z,A axes with various step distances

### Firmware Environment

The firmware builds using Teensyduino, the Arduino environment wrapper for the Teensy series of microcontroller boards.  A customized version of the Teensy 'cores' are required, as it contains the customized USB HID implementation necessary for the controls to work. 

This firmware was developed using the Linux version of the Arduino environment, but it should be possible to use any of the supported platforms with some changes to the process.

  - Extract "Legacy" Arduino 1.8.x into a directory specifically for this project, since the "cores" files will be modified.
  - Install the TeensyDuino add-ons from https://www.pjrc.com/teensy/td_download.html
  - Move aside the installed cores files and clone https://github.com/makeitlabs/teensyduino-cores (branch tormach-controls) in its place.  Be sure to check out that repo into the `cores` directory.
  - Back up the installed `boards.txt` file and replace it with the one in this repository
  - Clone this repo into your Arduino projects directory
  - Open the `.ino` file in the new modified Arduino environment
  - Select "Teensy LC" for the board and "Tormach Controls" for the USB type
  - Build code and program the Teensy
  
### Hardware Pinout

(This is based on the legacy hardware for the time being)

| Pin name | Pin number |
|----------|------------|
| PIN_BEACON_BLUE | 0 |
| PIN_BEACON_GREEN | 1 |
| PIN_BEACON_AMBER | 2 |
| PIN_BEACON_RED | 3 |
| PIN_BTN_START | 4 |
| PIN_BTN_STOP | 6 |
| PIN_BTN_FEED | 7 |
| PIN_BTN_M1 | 8 |
| PIN_ENCODER_A | 11 |
| PIN_ENCODER_B | 12 |
| PIN_LED_START | 5 |
| PIN_LED_FEED | 16 |
| PIN_LED_M1 | 17 |
| PIN_LED_RFID | 21 |


  
  

