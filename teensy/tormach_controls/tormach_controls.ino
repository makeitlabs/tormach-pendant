// work-in-progress R&D for new Tormach Operator Console emulation mode
//
// 2023-FEB-26 Steve Richardson (steve.richardson@makeitlabs.com)
//

#define ENCODER_USE_INTERRUPTS
#define ENCODER_OPTIMIZE_INTERRUPTS

#include <Encoder.h>
#include <Bounce.h>
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

#define PIN_JOG_A 14
#define PIN_JOG_B 15


#define NUM_RGB_LEDS 12
#define PIN_RGB_LED_DOUT 32


CRGB rgb_leds[NUM_RGB_LEDS];

Encoder enc_vel(PIN_ENCODER_V_A, PIN_ENCODER_V_B);
Encoder enc_feed(PIN_ENCODER_F_A, PIN_ENCODER_F_B);
Encoder enc_speed(PIN_ENCODER_S_A, PIN_ENCODER_S_B);

Bounce button_vel = Bounce(PIN_BTN_V, 10);
Bounce button_feed = Bounce(PIN_BTN_F, 10);
Bounce button_speed = Bounce(PIN_BTN_S, 10);



#define ENC_VEL_MAX 31
#define ENC_VEL_INIT 0
#define ENC_VEL_DEFAULT 0

#define ENC_FEED_MAX 200
#define ENC_FEED_INIT 100
#define ENC_FEED_DEFAULT 100

#define ENC_SPEED_MAX 200
#define ENC_SPEED_INIT 100
#define ENC_SPEED_DEFAULT 100


void setup() 
{
  pinMode(PIN_BTN_V, INPUT_PULLUP);
  pinMode(PIN_BTN_F, INPUT_PULLUP);
  pinMode(PIN_BTN_S, INPUT_PULLUP);

  Tormach.begin();
  Tormach.useManualSend(true);

  enc_vel.write(ENC_VEL_INIT);
  enc_feed.write(ENC_FEED_INIT);
  enc_speed.write(ENC_SPEED_INIT);

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

  const int vel_lut[] = {10,20,31,41,51,61,72,82,92,102,113,123,133,153,205,256,307,358,409,460,512,563,614,665,716,767,818,870,921,972,1023};
  
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
    //Serial.println(String("enc: V=") + cur_vel + String(" F=") + cur_feed + String(" S=") + cur_speed);
  }
 
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

void loop() 
{

  //led_poll();
  
  //bool button_update = button_poll();
  bool encoder_update = encoder_poll();
  
  //rgb_poll();

  if (encoder_update) {
    Tormach.send_now();
  }

}




  /*
  FastLED.addLeds<WS2812, PIN_RGB_LED_DOUT, GRB>(rgb_leds, NUM_RGB_LEDS);

  FastLED.setBrightness(255);
  for (int i=0; i< NUM_RGB_LEDS; i++) {
    rgb_leds[i] = CRGB::Black;
  }
  */


/*
void rgb_poll()
{
  static int value = 0;
  static bool flip = false;
  
  if (tic >= 200) {

    for (int i=0; i < NUM_RGB_LEDS; i++) {
      if (i < value) rgb_leds[i] = CRGB::Blue;
      else if (i == value) rgb_leds[i] = CRGB::White;
      else rgb_leds[i] = CRGB::Black;
    }

    FastLED.show();
    
    tic = 0;

    if (flip) {
      value--;
      if (value == 0) {
        flip = !flip;      
      }
    } else {
      value++;
      if (value == 11) {
        flip = !flip;
      }
    }
  }
}
*/

 
/*
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
     
    digitalWrite(PIN_LED_START, !(leds & 0x01));
    digitalWrite(PIN_LED_RFID, !(leds & 0x02));
    digitalWrite(PIN_BEACON_RED, !(leds & 0x04));
    digitalWrite(PIN_BEACON_GREEN, !(leds & 0x08));
    digitalWrite(PIN_BEACON_BLUE, !(leds & 0x10));

    return true;
  }
  return false;
}
*/

/*
bool button_poll() 
{
  uint8_t u = 0, uov = 0;

  button_start.update();
  button_stop.update();
  button_feed.update();
  button_status.update();
  button_fov.update();
  button_sov.update();
  
  if (button_start.fallingEdge()) { b_start = 1; u = 1; } else if (button_start.risingEdge()) { b_start = 0; u = 1;}
  if (button_feed.fallingEdge()) { b_feed = 1; u = 1;} else if (button_feed.risingEdge()) { b_feed = 0; u = 1;}
  if (button_fov.fallingEdge()) { b_fov = 1; uov = 1;} else if (button_fov.risingEdge()) { b_fov = 0; uov = 1;}
  if (button_sov.fallingEdge()) { b_sov = 1; uov = 1;} else if (button_sov.risingEdge()) { b_sov = 0; uov = 1;}

 // Additional possible functions via keyboard shortcuts:
 // F1 = peek at status tab
 // Esc or Alt-S = stop
 // Alt+Enter = MDI line focus
 // Alt+F = toggle Flood
 // Alt+M = toggle Mist

  // STOP is sent via emulated keyboard - ESC key
  if (button_stop.fallingEdge()) {
    b_stop = 1; 
    Keyboard.press(KEY_ESC);
    Keyboard.release(KEY_ESC);
  } else if (button_stop.risingEdge()) { 
    b_stop = 0; 
  }

  // STATUS PEEK is sent via emulated keyboard - F1 key
  if (button_status.fallingEdge()) {
    b_status = 1;
    Keyboard.press(KEY_F1);
  } else if (button_status.risingEdge()) {
    b_status = 0;
    Keyboard.release(KEY_F1);
  }



  // 'BTN_0': "button.feedhold",
  // 'BTN_1': "button.cycle-start",
  // 'BTN_2': "button.hold2run",
  // 'BTN_3': "switch.mode-select",
  // 'BTN_4': "switch.defeatured-mode",
  // 'BTN_5': "button.pendant-1",
  // 'BTN_6': "button.pendant-2",

  // FEED HOLD and START are sent via the HID descriptor
  Tormach.button(0, b_feed);
  Tormach.button(1, b_start);

    
 

  if (uov) {
    // B_FOV is the button that changes the encoder from MAXVEL to FEED OVERRIDE
    // B_SOV is the button that changes the encoder from MAXVEL to SPINDLE RPM OVERRIDE
    // when neither are pressed the encoder is MAXVEL (VOV)
    if (b_fov == 0 && b_sov == 0) 
      enc_val_idx = 0;
    else if (b_fov == 0 && b_sov == 1) 
      enc_val_idx = 1;
    else if (b_fov == 1 && b_sov == 0) 
      enc_val_idx = 2;
    else 
      enc_val_idx = 0;
  }
    
  return u==1;
}
*/
