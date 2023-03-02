//
// Tormach Operator Console USB HID emulation with enhanced features
//
// Steve Richardson (steve.richardson@makeitlabs.com)
//
//-------------------------------------------------------------------
// rgb_led.ino - RGB LED rings and status light

#include <WS2812Serial.h>
#define USE_WS2812SERIAL
#include <FastLED.h>

#define NUM_RGB_RING_LEDS 12
#define NUM_RGB_RINGS 3
#define NUM_RGB_STATUS_LEDS 16
#define STATUS_LED_START (NUM_RGB_RINGS * NUM_RGB_RING_LEDS)
#define NUM_RGB_LEDS ((NUM_RGB_RINGS * NUM_RGB_RING_LEDS) + NUM_RGB_STATUS_LEDS)
#define PIN_RGB_LED_DOUT 33

CRGB rgb_leds[NUM_RGB_LEDS];

void rgb_setup()
{
  LEDS.addLeds<WS2812SERIAL, PIN_RGB_LED_DOUT, BRG>(rgb_leds, NUM_RGB_LEDS);
  LEDS.setBrightness(255);

  for (int i=0; i< NUM_RGB_LEDS; i++) {
    rgb_leds[i] = CRGB::Black;
  }
}

void rgb_show()
{
  FastLED.show();
}

void rgb_status_update(bool rdy, bool board, bool red, bool green, bool blue)
{
  for (int i=2; i< NUM_RGB_STATUS_LEDS; i++) {
    rgb_leds[STATUS_LED_START + i].red = red ? 255 : 0;
    rgb_leds[STATUS_LED_START + i].green = green ? 255 : 0;
    rgb_leds[STATUS_LED_START + i].blue = blue ? 255 : 0;
  }

  rgb_leds[STATUS_LED_START] = rdy ? CHSV(224,255,255) : CHSV(0,0,0);
  rgb_leds[STATUS_LED_START + 1] = board ? CHSV(140,255,255) : CHSV(0,0,0);
  
  FastLED.show();
}

bool rgb_status_led_poll()
{
  int r = Tormach.update();
  
  if (r >= 0) {
    uint8_t leds = Tormach.leds();

    // 'LED_NUML': "led.ready",     0x01
    // 'LED_CAPSL': "led.board",    0x02
    // 'LED_SCROLLL': "led.red",    0x04
    // 'LED_COMPOSE': "led.green",  0x08
    // 'LED_KANA': "led.blue"       0x10

    rgb_status_update(leds & 0x01, leds & 0x02, leds & 0x04, leds & 0x08, leds & 0x10);
    
    return true;
  }
  return false;
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
