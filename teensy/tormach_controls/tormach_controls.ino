// Tormach Operator Console emulation mode with enhanced features
// 2023 version replaces the original version built in 2017
//
// 2023-FEB-26 Steve Richardson (steve.richardson@makeitlabs.com)
//

#define ENCODER_USE_INTERRUPTS
#define ENCODER_OPTIMIZE_INTERRUPTS

#include <Encoder.h>
#include <Bounce.h>
#include <WS2812Serial.h>
#define USE_WS2812SERIAL
#include <FastLED.h>

#define PIN_BTN_V 0
#define PIN_ENCODER_V_A 1
#define PIN_ENCODER_V_B 2

#define PIN_BTN_F 3
#define PIN_ENCODER_F_A 4
#define PIN_ENCODER_F_B 5

#define PIN_BTN_S 6
#define PIN_ENCODER_S_A 7
#define PIN_ENCODER_S_B 8

#define PIN_BTN_START 9
#define PIN_BTN_FEEDHOLD 10
#define PIN_BTN_STOP 11
#define PIN_BTN_COOLANT 12

#define NUM_RGB_LEDS 36
#define PIN_RGB_LED_DOUT 33

CRGB rgb_leds[NUM_RGB_LEDS];

Encoder enc_vel(PIN_ENCODER_V_A, PIN_ENCODER_V_B);
Encoder enc_feed(PIN_ENCODER_F_A, PIN_ENCODER_F_B);
Encoder enc_speed(PIN_ENCODER_S_A, PIN_ENCODER_S_B);

Bounce button_vel = Bounce(PIN_BTN_V, 10);
Bounce button_feed = Bounce(PIN_BTN_F, 10);
Bounce button_speed = Bounce(PIN_BTN_S, 10);

Bounce button_start = Bounce(PIN_BTN_START, 10);
Bounce button_feedhold = Bounce(PIN_BTN_FEEDHOLD, 10);
Bounce button_stop = Bounce(PIN_BTN_STOP, 10);
Bounce button_coolant = Bounce(PIN_BTN_COOLANT, 10);

#define ENC_VEL_MAX 32
#define ENC_VEL_INIT 0
#define ENC_VEL_DEFAULT 0

#define ENC_FEED_MAX 199
#define ENC_FEED_INIT 100
#define ENC_FEED_DEFAULT 100

#define ENC_SPEED_MAX 199
#define ENC_SPEED_INIT 100
#define ENC_SPEED_DEFAULT 100


void setup() 
{
  pinMode(PIN_BTN_V, INPUT_PULLUP);
  pinMode(PIN_BTN_F, INPUT_PULLUP);
  pinMode(PIN_BTN_S, INPUT_PULLUP);

  pinMode(PIN_BTN_START, INPUT_PULLUP);
  pinMode(PIN_BTN_FEEDHOLD, INPUT_PULLUP);
  pinMode(PIN_BTN_STOP, INPUT_PULLUP);
  pinMode(PIN_BTN_COOLANT, INPUT_PULLUP);

  enc_vel.write(ENC_VEL_INIT);
  enc_feed.write(ENC_FEED_INIT);
  enc_speed.write(ENC_SPEED_INIT);

  LEDS.addLeds<WS2812SERIAL, PIN_RGB_LED_DOUT, BRG>(rgb_leds, NUM_RGB_LEDS);
  LEDS.setBrightness(255);

  for (int i=0; i< NUM_RGB_LEDS; i++) {
    rgb_leds[i] = CRGB::Black;
  }

  Tormach.begin();
  Tormach.useManualSend(true);
}

bool encoder_limit(int &inval, int maxval)
{
  if (inval < 0) {
    inval = 0;
    return true;
  } else if (inval > maxval) {
    inval = maxval;
    return true;
  }
  return false;
}

bool encoder_poll()
{ 
  int cur_vel=0, cur_feed=0, cur_speed=0;
  static int last_vel=-1, last_feed=-1, last_speed=-1;
  bool updated = false;
  
  static elapsedMillis elapsed_vel_pressed;

  const int vel_lut[] = {  0, 39, 49, 59, 69, 79, 89, 99,109,119,129,
                         159,169,179,189,199,249,299,349,399,449,499,
                         549,599,649,699,749,799,849,899,949,974,999};
  
  // check for encoder shaft button press, which resets values to default
  button_vel.update();
  if (button_vel.fallingEdge()) {
    elapsed_vel_pressed=0;
  } else if (button_vel.risingEdge()) {
    if (elapsed_vel_pressed >= 500) {
      enc_vel.write(ENC_VEL_MAX);
    } else if (elapsed_vel_pressed > 100 && elapsed_vel_pressed < 500) {
      enc_vel.write(ENC_VEL_DEFAULT);
    }
  }

  button_feed.update();
  
  if (button_feed.fallingEdge()) {
    enc_feed.write(ENC_FEED_DEFAULT);
  }

  button_speed.update();
  if (button_speed.fallingEdge()) {
    enc_speed.write(ENC_SPEED_DEFAULT);
  }

  cur_vel = enc_vel.read();
  if (encoder_limit(cur_vel, ENC_VEL_MAX)) enc_vel.write(cur_vel);

  cur_feed = enc_feed.read();
  if (encoder_limit(cur_feed, ENC_FEED_MAX)) enc_feed.write(cur_feed);

  cur_speed = enc_speed.read();
  if (encoder_limit(cur_speed, ENC_SPEED_MAX)) enc_speed.write(cur_speed);

  if (cur_vel != last_vel || cur_feed != last_feed || cur_speed != last_speed) {
    last_vel = cur_vel;
    last_feed = cur_feed;
    last_speed = cur_speed;
    updated=true;

  }

  bool up_v = rgb_v_update(cur_vel);
  bool up_f = rgb_f_update(cur_feed);
  bool up_s = rgb_s_update(cur_speed);
  if (up_v || up_f || up_s)
    FastLED.show();
 
  // 0 - ABS_X - feed-override - "F%" confirmed
  // 1 - ABS_Y - rpm-override - "S%" confirmed
  // 2 - ABS_Z - rapid-override - "V%" confirmed (old "maxvel" equivalent)
  // 3 - ABS_RX - axis-select - no response
  // 4 - ABS_RY - step-select - no response
  // 5 - ABS_MISC - pendant-knob
  // 6 - ABS_WHEEL - jog-wheel
  // 7 - ABS_GAS - jog-wheel-2

  Tormach.absolute(0, (cur_feed * 1023) / ENC_FEED_MAX);
  Tormach.absolute(1, (cur_speed * 1023) / ENC_SPEED_MAX);
  Tormach.absolute(2, vel_lut[cur_vel]);

  Tormach.absolute(3, 1); // X axis
  Tormach.absolute(4, 1); // 0.001
  Tormach.absolute(5, 2); // knob=2
  Tormach.absolute(6, 0); // jog
  Tormach.absolute(7, 0); // jog-2
  
  return updated;
}

bool rgb_v_update(int cur_vel)
{
  static int last_cur_vel = -32768;
  static elapsedMillis elapsed;
  static uint16_t period = 250;
  static bool flipper = false;
  bool flipped = false;
  bool updated = false;
    
  if (elapsed >= period) {
    flipped = true;
    flipper = !flipper;
    elapsed = 0;
  }

  if (cur_vel != last_cur_vel || flipped) {
    updated = true;
    CHSV color0;
    CHSV color1;
    CHSV color2;
  
    
    int value = cur_vel;
  
    if (cur_vel >= 0 && cur_vel < 11) {
      // blue/violet - slowest speeds
      if (cur_vel == 0) {
        period = 500;
        color1 = flipper ? CHSV(0,255,255) : CHSV(170,255,255);
        color2 = CHSV(185,255,255);
        color0 = flipper ? CHSV(170,255,255) : CHSV(0,255,255);
      } else {
        period = 100;
        color1 = CHSV(170,255,128);
        color2 = flipper ? CHSV(185,255,255) : CHSV(0,255,255);
        color0 = color1;
      }
    } else if (cur_vel >= 11 && cur_vel < 22) {
      // yellow/orange - medium speeds
      period = 100;
      color1 = CHSV(36,255,192);
      color2 = flipper ? CHSV(64,255,255) : CHSV(64,255,64);
      color0 = color1;
      value -= 10;
    } else if (cur_vel >= 22 && cur_vel < 32) {
      // green/aqua - fast speeds
      period = 100;
      color1 = CHSV(96,255,128);
      color2 = flipper ? CHSV(128,255,255) : CHSV(128,255,64);
      color0 = color1;
      value -= 21;
    } else {
      period = 1000;
      color1 = CHSV(96,255,255);
      color2 = CHSV(96,255,255);
      color0 = color1;
    }
    
    for (int i=0; i < 12; i++) {
      if (i == 0) rgb_leds[i] = color0;
      else if (i != value) rgb_leds[i] = color1;
      else if (i == value) rgb_leds[i] = color2;
    }
  
    last_cur_vel = cur_vel;
  }
  return updated;
}


bool rgb_f_update(int cur_feed)
{
  static int last_cur_feed = -32768;
  
  static elapsedMillis elapsed;
  static uint16_t period = 250;
  static bool flipper = false;
  bool flipped = false;
  bool updated = false;
    
  if (elapsed >= period) {
    flipped = true;
    flipper = !flipper;
    elapsed = 0;
  }

  if (cur_feed != last_cur_feed || flipped) {
    updated = true;

    CHSV color0;
    CHSV color1;
    CHSV color2;

    int value = (cur_feed * 11 / ENC_FEED_MAX) + 1;
    
    color0 = CHSV(0,0,0);
    color1 = CHSV(0,0,0);
    color2 = CHSV(0,0, 255);
        
    for (int i=0; i < 12; i++) {
      if (i == 0) rgb_leds[i + 12] = color0;
      else if (i != value) rgb_leds[i + 12] = color1;
      else if (i == value) rgb_leds[i + 12] = color2;
    }
  
  
    last_cur_feed = cur_feed;
  }
  return updated;
}


bool rgb_s_update(int cur_speed)
{
  static int last_cur_speed = -32768;
  
  static elapsedMillis elapsed;
  static uint16_t period = 250;
  static bool flipper = false;
  bool flipped = false;
  bool updated = false;
  
  if (elapsed >= period) {
    flipped = true;
    flipper = !flipper;
    elapsed = 0;
  }

  if (cur_speed != last_cur_speed || flipped) {
    updated = true;
    CHSV color0;
    CHSV color1;
    CHSV color2;

    int value = (cur_speed * 11 / ENC_FEED_MAX) + 1;
    
    color0 = CHSV(0,0,0);
    color1 = CHSV(0,0,0);
    color2 = CHSV(0,0,255);
        
    for (int i=0; i < 12; i++) {
      if (i == 0) rgb_leds[i + 24] = color0;
      else if (i != value) rgb_leds[i + 24] = color1;
      else if (i == value) rgb_leds[i + 24] = color2;
    }
  
  
    last_cur_speed = cur_speed;
  }

  return updated;
}



bool led_poll()
{
  int r = Tormach.update();
  
  if (r >= 0) {
    uint8_t leds = Tormach.leds();

    // 'LED_NUML': "led.ready",     0x01
    // 'LED_CAPSL': "led.board",    0x02
    // 'LED_SCROLLL': "led.red",    0x04
    // 'LED_COMPOSE': "led.green",  0x08
    // 'LED_KANA': "led.blue"       0x10

    /*
    digitalWrite(PIN_LED_START, !(leds & 0x01));
    digitalWrite(PIN_LED_RFID, !(leds & 0x02));
    digitalWrite(PIN_BEACON_RED, !(leds & 0x04));
    digitalWrite(PIN_BEACON_GREEN, !(leds & 0x08));
    digitalWrite(PIN_BEACON_BLUE, !(leds & 0x10));
    */
    
    return true;
  }
  return false;
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



void loop() 
{
  static elapsedMillis elapsed;

  led_poll();
  
  bool button_update = button_poll();
  bool encoder_update = encoder_poll();
  
  if (button_update || encoder_update || elapsed >= 1000) {
    elapsed = 0;
    Tormach.send_now();
  }
}
