//
// Tormach Operator Console USB HID emulation with enhanced features
//
// Steve Richardson (steve.richardson@makeitlabs.com)
//

void setup() 
{
  button_setup();
  encoder_setup();
  rgb_setup();
  serial_setup();  
  
  Tormach.begin();
  Tormach.useManualSend(true);
}

void loop() 
{
  static elapsedMillis elapsed;

  bool led_update = rgb_status_led_poll();
  
  bool button_update = button_poll();
  bool encoder_update = encoder_poll();
  bool mpg_update = serial_poll();
  
  if (button_update || encoder_update|| mpg_update || led_update) {
    elapsed = 0;
    Tormach.send_now();
  }
}
