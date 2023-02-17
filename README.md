# tormach-pendant

Support for custom USB Pendant for the Tormach CNC mill with PathPilot 1.x control.

This is the legacy implementation that required heavy customization to PathPilot code, and was used at Makeit Labs from 2016-early 2023.  It is being replaced with a newer version that emulates the official Tormach PathPilot Operator Console, to eliminate the need for the tedious customization process.

Physical pendant based on Teensy LC:
  - physical CYCLE START, FEED HOLD, STOP, M01 BREAK buttons
  - physical MAXVEL rotary encoder for better control of speeds when proving new G-Code
  - LED feedback for state of FEED HOLD, M01 BREAK, CYCLE START
  - stack light support for up to 4 lights (BLUE/GREEN/YELLOW/RED) - e.g. error, cycle start, etc.
  - RFID reader interface for access control


