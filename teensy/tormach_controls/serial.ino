//
// Tormach Operator Console USB HID emulation with enhanced features
//
// Steve Richardson (steve.richardson@makeitlabs.com)
//
//-------------------------------------------------------------------
// serial.ino - receives serial data from the Teensy in the MPG pendant

#define RX_BUFLEN 64

enum SerialStates {
  STATE_SER_IDLE = 0,
  STATE_SER_BTN,
  STATE_SER_BTN_DONE,
  STATE_SER_AXIS,
  STATE_SER_AXIS_DONE,
  STATE_SER_INC,
  STATE_SER_INC_DONE,
  STATE_SER_MPG,
  STATE_SER_PARSED
};

void serial_setup()
{
  Serial4.begin(115200);
}


bool serial_poll()
{
  static char rx_buf[RX_BUFLEN];
  static uint8_t rx_idx = 0;
  static uint8_t state = STATE_SER_IDLE;

  static bool r_btn;
  static char r_axis;
  static char r_inc;
  static int r_mpg;
  
  while (Serial4.available() > 0) {
    char rx_byte = Serial4.read();
    switch(state) {
      case STATE_SER_IDLE:
        state = STATE_SER_BTN;
        // fallthrough
      case STATE_SER_BTN:
        if (rx_byte == 'b') {
          r_btn = false;
          state = STATE_SER_BTN_DONE;
        } else if (rx_byte == 'B') {
          r_btn = true;
          state = STATE_SER_BTN_DONE;
        } else {
          state = STATE_SER_IDLE;
        }
        break;
      case STATE_SER_BTN_DONE:
        if (rx_byte == ',') {
          state = STATE_SER_AXIS;
        } else {
          state = STATE_SER_IDLE;
        }
        break;
      case STATE_SER_AXIS:
        if (rx_byte == 'X') {
          r_axis = 1;
          state = STATE_SER_AXIS_DONE;
        } else if (rx_byte == 'Y') {
          r_axis = 2;
          state = STATE_SER_AXIS_DONE;
        } else if (rx_byte == 'Z') {
          r_axis = 3;
          state = STATE_SER_AXIS_DONE;
        } else {
          state = STATE_SER_IDLE;
        }
        break;
      case STATE_SER_AXIS_DONE:
        if (rx_byte == ',') {
          state = STATE_SER_INC;
        } else {
          state = STATE_SER_IDLE;
        }
        break;
      case STATE_SER_INC:
        if (rx_byte == 'S') {
          r_inc = 0;
          state = STATE_SER_INC_DONE;
        } else if (rx_byte == 'M') {
          r_inc = 1;
          state = STATE_SER_INC_DONE;
        } else if (rx_byte == 'L') {
          r_inc = 2;
          state = STATE_SER_INC_DONE;
        } else {
          state = STATE_SER_IDLE;
        }
        break;
      case STATE_SER_INC_DONE:
        if (rx_byte == ',') {
          state = STATE_SER_MPG;
        } else {
          state = STATE_SER_IDLE;
        }
        break;
      case STATE_SER_MPG:
        if (rx_idx < RX_BUFLEN-2 && rx_byte != '\n' && rx_byte != '\r') {
          rx_buf[rx_idx++] = rx_byte;
        } else if (rx_byte == '\n') {
          rx_buf[rx_idx] = '\0';
          rx_idx = 0;

          r_mpg = atoi(rx_buf);
          
          state = STATE_SER_PARSED;
        }
        break;
      case STATE_SER_PARSED:
        state = STATE_SER_IDLE;

        // 3 - ABS_RX - axis-select
        // 4 - ABS_RY - step-select
        // 5 - ABS_MISC - pendant-knob
        // 6 - ABS_WHEEL - jog-wheel
        // 7 - ABS_GAS - jog-wheel-2
        Tormach.absolute(3, r_axis);
        Tormach.absolute(4, r_inc);
        Tormach.absolute(5, 0); // knob - unsure of use
        Tormach.absolute(6, r_mpg);
        Tormach.absolute(7, 0); // jog-2 - unsure of use

        // 'BTN_5': "button.pendant-1",
        Tormach.button(5, r_btn);
        
        return true;
        break;
    }
  }
  return false;
}
