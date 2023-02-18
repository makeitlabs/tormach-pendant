// work-in-progress R&D for new Tormach Operator Console emulation mode
//
// 2023-FEB-18 Steve Richardson (steve.richardson@makeitlabs.com)
//

#define ENCODER_USE_INTERRUPTS
#define ENCODER_OPTIMIZE_INTERRUPTS

#include <Encoder.h>
#include <Bounce.h>

#define PIN_BEACON_BLUE 0
#define PIN_BEACON_GREEN 1
#define PIN_BEACON_AMBER 2
#define PIN_BEACON_RED 3

#define PIN_BTN_START 4
#define PIN_BTN_STOP 6
#define PIN_BTN_FEED 7
#define PIN_BTN_M1 8

#define PIN_BTN_FOV 10
#define PIN_BTN_SOV 18

#define PIN_ENCODER_A 11
#define PIN_ENCODER_B 12

#define PIN_JOG_A 14
#define PIN_JOG_B 15

#define PIN_LED_START 5
#define PIN_LED_FEED 16
#define PIN_LED_M1 17
#define PIN_LED_RFID 21

Encoder encoder(PIN_ENCODER_A, PIN_ENCODER_B);
Encoder jog(PIN_JOG_A, PIN_JOG_B);

Bounce button_start = Bounce(PIN_BTN_START, 10);
Bounce button_stop = Bounce(PIN_BTN_STOP, 10);
Bounce button_feed = Bounce(PIN_BTN_FEED, 10);
Bounce button_m1 = Bounce(PIN_BTN_M1, 10);
Bounce button_fov = Bounce(PIN_BTN_FOV, 10);
Bounce button_sov = Bounce(PIN_BTN_SOV, 10);

static bool b_start = false, b_stop = false, b_feed = false, b_m1 = false, b_fov = false, b_sov = false;

static uint16_t enc_vals[3] = { 999, 500, 500 };
static uint8_t enc_val_idx = 0;

#define ENC_IDX_VOV 0
#define ENC_IDX_SOV 1
#define ENC_IDX_FOV 2

void setup() 
{
 
  pinMode(PIN_BTN_START, INPUT_PULLUP);
  pinMode(PIN_BTN_STOP, INPUT_PULLUP);
  pinMode(PIN_BTN_FEED, INPUT_PULLUP);
  pinMode(PIN_BTN_M1, INPUT_PULLUP);
  pinMode(PIN_BTN_FOV, INPUT_PULLUP);
  pinMode(PIN_BTN_SOV, INPUT_PULLUP);

  //pinMode(PIN_ENCODER_A, INPUT_PULLUP);
  //pinMode(PIN_ENCODER_B, INPUT_PULLUP);
  //pinMode(PIN_JOG_A, INPUT_PULLUP);
  //pinMode(PIN_JOG_B, INPUT_PULLUP);
  
  pinMode(PIN_LED_START, OUTPUT);
  pinMode(PIN_LED_FEED, OUTPUT);
  pinMode(PIN_LED_M1, OUTPUT);
  pinMode(PIN_LED_RFID, OUTPUT);
  pinMode(PIN_BEACON_GREEN, OUTPUT);
  pinMode(PIN_BEACON_AMBER, OUTPUT);
  pinMode(PIN_BEACON_RED, OUTPUT);
  pinMode(PIN_BEACON_BLUE, OUTPUT);

  digitalWrite(PIN_LED_START, 1);
  digitalWrite(PIN_LED_FEED, 1);
  digitalWrite(PIN_LED_M1, 1);
  digitalWrite(PIN_LED_RFID, 1);
  
  digitalWrite(PIN_BEACON_GREEN, 1);
  digitalWrite(PIN_BEACON_AMBER, 1);
  digitalWrite(PIN_BEACON_RED, 1);
  digitalWrite(PIN_BEACON_BLUE, 1);

  Tormach.begin();
  Tormach.useManualSend(true);

  encoder.write(500);
  jog.write(1);
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
     
    digitalWrite(PIN_LED_START, !(leds & 0x01));
    digitalWrite(PIN_LED_RFID, !(leds & 0x02));
    digitalWrite(PIN_BEACON_BLUE, !(leds & 0x10));
    digitalWrite(PIN_BEACON_GREEN, !(leds & 0x08));
    digitalWrite(PIN_BEACON_RED, !(leds & 0x04));

    return true;
  }
  return false;
}

bool button_poll() 
{
  uint8_t u = 0, uov = 0;

  button_start.update();
  button_stop.update();
  button_feed.update();
  button_m1.update();
  button_fov.update();
  button_sov.update();
  
  if (button_start.fallingEdge()) { b_start = 1; u = 1; } else if (button_start.risingEdge()) { b_start = 0; u = 1;}
  if (button_stop.fallingEdge()) { b_stop = 1; u = 1;} else if (button_stop.risingEdge()) { b_stop = 0; u = 1;}
  if (button_feed.fallingEdge()) { b_feed = 1; u = 1;} else if (button_feed.risingEdge()) { b_feed = 0; u = 1;}
  if (button_m1.fallingEdge()) { b_m1 = 1; u = 1;} else if (button_m1.risingEdge()) { b_m1 = 0; u = 1;}
  if (button_fov.fallingEdge()) { b_fov = 1; uov = 1;} else if (button_fov.risingEdge()) { b_fov = 0; uov = 1;}
  if (button_sov.fallingEdge()) { b_sov = 1; uov = 1;} else if (button_sov.risingEdge()) { b_sov = 0; uov = 1;}

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

bool encoder_poll()
{ 
  static uint8_t last_enc_val_idx = 255;
  
  static int16_t last_jog_val = -1;
  int16_t enc_val, jog_val;
  
  bool updated = false;
  
  if (enc_val_idx != last_enc_val_idx) {
    encoder.write(enc_vals[enc_val_idx]);
    //Serial.println(String("enc_val_idx: ") + enc_val_idx);
  }
  last_enc_val_idx = enc_val_idx;

  // read the encoder value, limit it to 0-999, save it
  enc_val = encoder.read();
  if (enc_val < 0) {
    enc_val = 0;
  } else if (enc_val > 999) {
    enc_val = 999;
  }

  if (enc_vals[enc_val_idx] != enc_val) {
    enc_vals[enc_val_idx] = enc_val;
    updated = true;
    //Serial.println(String("enc: ") + enc_val);
  }


  jog_val = jog.read();
  
  if (last_jog_val != jog_val) {
    last_jog_val = jog_val;
    updated = true;
    //Serial.println(String("jog: ") + jog_val);
  }


  // 0 - ABS_X - feed-override - "F%" confirmed
  // 1 - ABS_Y - rpm-override - "S%" confirmed
  // 2 - ABS_Z - rapid-override - "V%" confirmed (old "maxvel" equivalent)
  // 3 - ABS_RX - axis-select - no response
  // 4 - ABS_RY - step-select - no response
  // 5 - ABS_MISC - pendant-knob
  // 6 - ABS_WHEEL - jog-wheel
  // 7 - ABS_GAS - jog-wheel-2

  Tormach.absolute(0, enc_vals[ENC_IDX_FOV]);
  Tormach.absolute(1, enc_vals[ENC_IDX_SOV]);
  Tormach.absolute(2, enc_vals[ENC_IDX_VOV]);

  Tormach.absolute(3, 1); // X axis
  Tormach.absolute(4, 1); // 0.001
  Tormach.absolute(5, 2); // knob=2
  Tormach.absolute(6, jog_val);
  Tormach.absolute(7, 0); // jog-2
  
  return updated;
}

void loop() 
{

  led_poll();
  
  bool button_update = button_poll();
  bool encoder_update = encoder_poll();
  
  if (button_update || encoder_update) {
    Tormach.send_now();
  }

}

/*
 * Additional possible functions via keyboard shortcuts:
 * 
 * F1 = peek at status tab
 * 
 * Esc or Alt-S = stop
 * 
 * Alt+Enter = MDI line focus
 * 
 * Alt+F = toggle Flood
 * Alt+M = toggle Mist
 * 
 * 
 */
