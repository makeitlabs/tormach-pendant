// Tormach Operator Console emulation with enhanced features
//
// 2023-MAR-01 Steve Richardson (steve.richardson@makeitlabs.com)
//
// Serial output MPG pendant for Teensy LC:
// - 100PPR Manual Pulse Generator (quadrature encoder)
// - Two 3-way rotary switches for Axis X/Y/Z, Increment 0.0001/0.0010/0.0100"
// - Momentary pushbutton for safety switch to ignore MPG input unless pressed
//
// Serial port runs at 115200 baud, 8-N-1.
// Serial output format is ASCII text, comma delimited and terminated with CR/LF:
//
// Button,Axis,Increment,MPG
//
// Button ('B' - pressed, 'b' - not pressed)
// Axis ('X', 'Y', or 'Z' and '-' indicates an error state)
// Increment ('S' for small, 'M' for medium, 'L' for large, and '-' indicates an error state)
// MPG current signed 16 bit int value of encoder
//
// A new data line is sent any time a value changes.
//

#define ENCODER_USE_INTERRUPTS
#define ENCODER_OPTIMIZE_INTERRUPTS

#include <Encoder.h>
#include <Bounce.h>

// MPG quadrature A/B inputs.  Note many MPGs are 5V and the Teensy LC is not 5V tolerant.  
// Use a voltage divider or other method for level shift.
#define PIN_MPG_A 3
#define PIN_MPG_B 4

// Inputs for the two 'bits' of the normally open active low 3-way selector switches
#define PIN_SEL_AXIS_0 15
#define PIN_SEL_AXIS_1 16

#define PIN_SEL_INC_0 17
#define PIN_SEL_INC_1 18

// Input for the normally open active low momentary pushbutton switch
#define PIN_BTN 19

// Teensy LC internal LED
#define PIN_LED 13


Encoder enc_mpg(PIN_MPG_A, PIN_MPG_B);

Bounce sel_axis_0 = Bounce(PIN_SEL_AXIS_0, 10);
Bounce sel_axis_1 = Bounce(PIN_SEL_AXIS_1, 10);
Bounce sel_inc_0 = Bounce(PIN_SEL_INC_0, 10);
Bounce sel_inc_1 = Bounce(PIN_SEL_INC_1, 10);
Bounce btn = Bounce(PIN_BTN, 10);

void setup() {
  Serial1.begin(115200);
  pinMode(PIN_LED, OUTPUT);
  pinMode(PIN_SEL_AXIS_0, INPUT_PULLUP);
  pinMode(PIN_SEL_AXIS_1, INPUT_PULLUP);
  pinMode(PIN_SEL_INC_0, INPUT_PULLUP);
  pinMode(PIN_SEL_INC_1, INPUT_PULLUP);
  pinMode(PIN_BTN, INPUT_PULLUP);
}

void loop() {
  static int last_mpg = -1;
  static uint8_t bit_a0=digitalRead(PIN_SEL_AXIS_0), bit_a1=digitalRead(PIN_SEL_AXIS_1), bit_i0=digitalRead(PIN_SEL_INC_0), bit_i1=digitalRead(PIN_SEL_INC_1);
  int aup=0, iup=0, bup=0;
  const char axis_lut[] = { '-', 'Z', 'X', 'Y' };
  const char inc_lut[] = { '-', 'L', 'S', 'M' };
  uint8_t axis_sel, inc_sel;
  bool btn_pressed = digitalRead(PIN_BTN) == 0;
  static elapsedMillis elapsed;
  static bool led;
      
  int cur_mpg = enc_mpg.read();

  btn.update();
  if (btn.risingEdge()) { btn_pressed=false; bup=1; } else if (btn.fallingEdge()) { btn_pressed=true; bup=1; }

  sel_axis_0.update();
  sel_axis_1.update();
  
  sel_inc_0.update();
  sel_inc_1.update();

  if (sel_axis_0.risingEdge()) { bit_a0=1; aup=1; } else if (sel_axis_0.fallingEdge()) { bit_a0=0; aup=1; }
  if (sel_axis_1.risingEdge()) { bit_a1=1; aup=1; } else if (sel_axis_1.fallingEdge()) { bit_a1=0; aup=1; }
  if (sel_inc_0.risingEdge()) { bit_i0=1; iup=1; } else if (sel_inc_0.fallingEdge()) { bit_i0=0; iup=1; }
  if (sel_inc_1.risingEdge()) { bit_i1=1; iup=1; } else if (sel_inc_1.fallingEdge()) { bit_i1=0; iup=1; }

  axis_sel = (bit_a1 << 1) | bit_a0;
  inc_sel = (bit_i1 << 1) | bit_i0;
   
  if (aup || iup || bup || cur_mpg != last_mpg) {
    Serial1.println(String() + (btn_pressed ? "B," : "b,") + axis_lut[axis_sel] + "," + inc_lut[inc_sel] + "," + cur_mpg);
    last_mpg = cur_mpg;
  }

  if (elapsed >= 500) {
    digitalWrite(PIN_LED, led);
    led = !led;    
    elapsed = 0;
  }
}
