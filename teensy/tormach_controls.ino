// work-in-progress R&D for new Tormach Controls
// 2023-FEB-16 steve.richardson@makeitlabs.com
// using legacy hardware pin mapping to test new HID interface code

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

#define PIN_ENCODER_A 11
#define PIN_ENCODER_B 12

#define PIN_LED_START 5
#define PIN_LED_FEED 16
#define PIN_LED_M1 17
#define PIN_LED_RFID 21

Encoder encoder_a(PIN_ENCODER_A, PIN_ENCODER_B);

Bounce button_start = Bounce(PIN_BTN_START, 10);
Bounce button_stop = Bounce(PIN_BTN_STOP, 10);
Bounce button_feed = Bounce(PIN_BTN_FEED, 10);
Bounce button_m1 = Bounce(PIN_BTN_M1, 10);

void setup() {
 
  pinMode(PIN_BTN_START, INPUT_PULLUP);
  pinMode(PIN_BTN_STOP, INPUT_PULLUP);
  pinMode(PIN_BTN_FEED, INPUT_PULLUP);
  pinMode(PIN_BTN_M1, INPUT_PULLUP);

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

  encoder_a.write(480);
}

void loop() {
  static bool b0 = false, b1 = false, b2 = false, b3 = false;
  static bool lb0 = true, lb1 = true, lb2 = true, lb3 = true;
  static int16_t lenc_a;
  static uint8_t enc_f = 0;
  static uint16_t encs[8] = {0,0,0,0,0,0,0,0};

  static unsigned long lmilli = 0;

  unsigned long milli = millis();

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
  }
  
  button_start.update();
  button_stop.update();
  button_feed.update();
  button_m1.update();
  
  if (button_start.fallingEdge()) {
    b0 = 1;
    enc_f++;
    if (enc_f == 8)
      enc_f = 0;

    encoder_a.write(encs[enc_f]);

    Serial.println(String("Switch to: ") + enc_f);
    
  } else if (button_start.risingEdge()) {
    b0 = 0;
  }
  
  if (button_stop.fallingEdge()) {
    b1 = 1;
  } else if (button_stop.risingEdge()) {
    b1 = 0;
  }
  
  if (button_feed.fallingEdge()) {
    b2 = 1;
  } else if (button_feed.risingEdge()) {
    b2 = 0;
  }
  
  if (button_m1.fallingEdge()) {
    b3 = 1;
  } else if (button_m1.risingEdge()) {
    b3 = 0;
  }
    
  int16_t enc_a = encoder_a.read();

  if (enc_a < 0) {
    encoder_a.write(0);
    enc_a = 0;
  } else if (enc_a > 959) {
    encoder_a.write(959);
    enc_a = 959;
  }

  
  if (b0 != lb0 || b1 != lb1 || b2 != lb2 || b3 != lb3 || lenc_a != enc_a) {

    // 'BTN_0': "button.feedhold",
    // 'BTN_1': "button.cycle-start",
    // 'BTN_2': "button.hold2run",
    // 'BTN_3': "switch.mode-select",
    // 'BTN_4': "switch.defeatured-mode",
    // 'BTN_5': "button.pendant-1",
    // 'BTN_6': "button.pendant-2",
    
    //Tormach.button(0, b0);
    Tormach.button(1, b1);

    Tormach.button(5, b2);
    Tormach.button(6, b3);

    
    // 0 - ABS_X - feed-override - "F%" confirmed
    // 1 - ABS_Y - rpm-override - "S%" confirmed
    // 2 - ABS_Z - rapid-override - "V%" confirmed (old "maxvel" equivalent)
    // 3 - ABS_RX - axis-select - no response
    // 4 - ABS_RY - step-select - no response
    // 5 - ABS_MISC - pendant-knob
    // 6 - ABS_WHEEL - jog-wheel
    // 7 - ABS_GAS - jog-wheel-2
    //Tormach.absolute(0, 500);
    //Tormach.absolute(1, 500);
    //Tormach.absolute(2, 1000);

    for (uint8_t i=0;i<8;i++) {
      encs[enc_f] = enc_a;
      
      Tormach.absolute(i, encs[i]);

      Serial.print(String("[") + i + "=" + encs[i] + "]");
    }
    Serial.println();
    

    Tormach.send_now();
    
    lb0 = b0;
    lb1 = b1;
    lb2 = b2;
    lb3 = b3;
    lenc_a = enc_a;
    lmilli = milli;
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



/*  since ABS values used, the controller can do overrides: e.g, 0%, 50%, 100% direct via buttons.
 *   
    if (b3) {
      encoder_a.write(1000);
    }
*/
