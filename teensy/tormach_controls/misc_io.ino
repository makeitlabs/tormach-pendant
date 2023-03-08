//
// Tormach Operator Console USB HID emulation with enhanced features
//
// Steve Richardson (steve.richardson@makeitlabs.com)
//
//-------------------------------------------------------------------
// misc_io.ino - miscellaneous I/O functions, mostly RATT integration

#include <Bounce.h>


#define PIN_IN_TOOL_ENABLED 34
#define PIN_IN_SPINDLE_ENABLED 35
#define PIN_LED_SPINDLE_LOCK 36

Bounce in_tool_enabled = Bounce(PIN_IN_TOOL_ENABLED, 10);
Bounce in_spindle_enabled = Bounce(PIN_IN_SPINDLE_ENABLED, 10);

bool tool_enabled = false;
bool spindle_enabled = false;

void misc_io_setup()
{
  pinMode(PIN_IN_TOOL_ENABLED, INPUT_PULLUP);
  pinMode(PIN_IN_SPINDLE_ENABLED, INPUT_PULLUP);
  pinMode(PIN_LED_SPINDLE_LOCK, OUTPUT);

  tool_enabled = !digitalRead(PIN_IN_TOOL_ENABLED);
  spindle_enabled = !digitalRead(PIN_IN_SPINDLE_ENABLED);
}

void misc_io_poll()
{
  static elapsedMillis elapsed;
  static uint16_t period = 500;
  static bool flipper = false;
  bool u = false;
  bool flipped = false;
  
  in_tool_enabled.update();
  in_spindle_enabled.update();
  if (in_tool_enabled.risingEdge()) { tool_enabled = 0; u = 1; } else if (in_tool_enabled.fallingEdge()) { tool_enabled = 1; u = 1;}
  if (in_spindle_enabled.risingEdge()) { spindle_enabled = 0; u = 1; } else if (in_spindle_enabled.fallingEdge()) { spindle_enabled = 1; u = 1;}
    
  if (elapsed >= period) {
    flipper = !flipper;
    elapsed = 0;
    flipped = true;
  }

  if (flipped || u) {
    if (tool_enabled && !spindle_enabled) {
      digitalWrite(PIN_LED_SPINDLE_LOCK, flipper);
    } else {
      digitalWrite(PIN_LED_SPINDLE_LOCK, HIGH);  
    }
  }
}
