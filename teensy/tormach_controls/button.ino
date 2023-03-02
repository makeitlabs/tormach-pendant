//
// Tormach Operator Console USB HID emulation with enhanced features
//
// Steve Richardson (steve.richardson@makeitlabs.com)
//
//-------------------------------------------------------------------
// buttons.ino - Debounced button inputs

#include <Bounce.h>

#define PIN_BTN_START 9
#define PIN_BTN_FEEDHOLD 10
#define PIN_BTN_STOP 11
#define PIN_BTN_COOLANT 12

Bounce button_start = Bounce(PIN_BTN_START, 10);
Bounce button_feedhold = Bounce(PIN_BTN_FEEDHOLD, 10);
Bounce button_stop = Bounce(PIN_BTN_STOP, 10);
Bounce button_coolant = Bounce(PIN_BTN_COOLANT, 10);


void button_setup()
{
  pinMode(PIN_BTN_START, INPUT_PULLUP);
  pinMode(PIN_BTN_FEEDHOLD, INPUT_PULLUP);
  pinMode(PIN_BTN_STOP, INPUT_PULLUP);
  pinMode(PIN_BTN_COOLANT, INPUT_PULLUP);
}

/* 
 * Note that the button inputs are pulled up and active low, however they are connected to normally closed pushbuttons, essentially making them active high
 * This is done in hopes of improved noise immunity, as the inputs are tied to ground in normal state
 */
bool button_poll() 
{
  uint8_t u = 0;
  static uint8_t b_start=0, b_feedhold=0;

  button_start.update();
  button_feedhold.update();
  button_stop.update();
  button_coolant.update();
  
  if (button_start.risingEdge()) { b_start = 1; u = 1; } else if (button_start.fallingEdge()) { b_start = 0; u = 1;}
  if (button_feedhold.risingEdge()) { b_feedhold = 1; u = 1;} else if (button_feedhold.fallingEdge()) { b_feedhold = 0; u = 1;}

 // Additional possible functions via keyboard shortcuts:
 // F1 = peek at status tab
 // Esc or Alt-S = stop
 // Alt+Enter = MDI line focus
 // Alt+F = toggle Flood
 // Alt+M = toggle Mist

  // STOP is sent via emulated keyboard - ESC key
  if (button_stop.risingEdge()) {
    Keyboard.set_modifier(0);
    Keyboard.send_now();
    Keyboard.set_key1(KEY_ESC);
    Keyboard.send_now();
    delay(50);
    Keyboard.set_key1(0);
    Keyboard.send_now();
  }

  // COOLANT TOGGLE is sent via emulated keyboard - Alt+M for mist and Alt+F for flood
  if (button_coolant.risingEdge()) {
    Keyboard.set_modifier(MODIFIERKEY_ALT);
    Keyboard.send_now();
    Keyboard.set_key1(KEY_F);
    Keyboard.send_now();
    delay(50);
    Keyboard.set_key1(0);
    Keyboard.send_now();
    Keyboard.set_modifier(0);
    Keyboard.send_now();
  }
  
  // 'BTN_0': "button.feedhold",
  // 'BTN_1': "button.cycle-start",
  // 'BTN_2': "button.hold2run",
  // 'BTN_3': "switch.mode-select",
  // 'BTN_4': "switch.defeatured-mode",
  // 'BTN_5': "button.pendant-1",
  // 'BTN_6': "button.pendant-2",

  // FEED HOLD and START are sent via the HID descriptor
  Tormach.button(0, b_feedhold);
  Tormach.button(1, b_start);
    
  return u==1;
}
