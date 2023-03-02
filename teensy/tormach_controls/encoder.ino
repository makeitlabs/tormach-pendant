//
// Tormach Operator Console USB HID emulation with enhanced features
//
// Steve Richardson (steve.richardson@makeitlabs.com)
//
//-------------------------------------------------------------------
// encoder.ino - three quadrature encoders with buttons

#define ENCODER_USE_INTERRUPTS
#define ENCODER_OPTIMIZE_INTERRUPTS

#include <Encoder.h>
#include <Bounce.h>

#define PIN_BTN_V 0
#define PIN_ENCODER_V_A 1
#define PIN_ENCODER_V_B 2

#define PIN_BTN_F 3
#define PIN_ENCODER_F_A 4
#define PIN_ENCODER_F_B 5

#define PIN_BTN_S 6
#define PIN_ENCODER_S_A 7
#define PIN_ENCODER_S_B 8

Encoder enc_vel(PIN_ENCODER_V_A, PIN_ENCODER_V_B);
Encoder enc_feed(PIN_ENCODER_F_A, PIN_ENCODER_F_B);
Encoder enc_speed(PIN_ENCODER_S_A, PIN_ENCODER_S_B);

Bounce button_vel = Bounce(PIN_BTN_V, 10);
Bounce button_feed = Bounce(PIN_BTN_F, 10);
Bounce button_speed = Bounce(PIN_BTN_S, 10);

#define ENC_VEL_MAX 32
#define ENC_VEL_INIT 0
#define ENC_VEL_DEFAULT 0

#define ENC_FEED_MAX 199
#define ENC_FEED_INIT 100
#define ENC_FEED_DEFAULT 100

#define ENC_SPEED_MAX 199
#define ENC_SPEED_INIT 100
#define ENC_SPEED_DEFAULT 100


void encoder_setup()
{
  pinMode(PIN_BTN_V, INPUT_PULLUP);
  pinMode(PIN_BTN_F, INPUT_PULLUP);
  pinMode(PIN_BTN_S, INPUT_PULLUP);

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
    rgb_show();
 
  // 0 - ABS_X - feed-override - "F%" confirmed
  // 1 - ABS_Y - rpm-override - "S%" confirmed
  // 2 - ABS_Z - rapid-override - "V%" confirmed (old "maxvel" equivalent)

  Tormach.absolute(0, (cur_feed * 1023) / ENC_FEED_MAX);
  Tormach.absolute(1, (cur_speed * 1023) / ENC_SPEED_MAX);
  Tormach.absolute(2, vel_lut[cur_vel]);

  
  return updated;
}
