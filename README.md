# tormach-pendant
Support for custom USB Pendant for the Tormach CNC mill with PathPilot control

Physical pendant based on Teensy LC:
  - physical CYCLE START, FEED HOLD, STOP, M01 BREAK buttons
  - physical MAXVEL rotary encoder for better control of speeds when proving new G-Code
  - LED feedback for state of FEED HOLD, M01 BREAK, CYCLE START
  - stack light support for up to 4 lights (BLUE/GREEN/YELLOW/RED) - e.g. error, cycle start, etc.
  - RFID reader interface for access control

To be useful, requires a number of changes to PathPilot itself, also covered in this repository.

