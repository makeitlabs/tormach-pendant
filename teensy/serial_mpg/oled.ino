
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <Fonts/FreeSansBold24pt7b.h>
#include <Fonts/FreeSans9pt7b.h>
#include <Fonts/FreeSansBold9pt7b.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32

Adafruit_SH1106G display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire1);

void oled_setup()
{
  display.begin(SCREEN_ADDRESS, true);
  splash();
}

void splash(void) {
  display.clearDisplay();

  display.fillRect(3, 0, 121, 36, SH110X_WHITE);

  display.setFont(&FreeSansBold9pt7b);
  display.setTextSize(1);
  display.setTextColor(SH110X_BLACK);

  display.setCursor(0, 15);
  display.print(F("  MakeIt Labs"));

  display.setFont(&FreeSans9pt7b);
  display.setCursor(0, 32);
  display.print(F(" Tormach 1100"));

  display.setTextColor(SH110X_WHITE);

  display.setFont();
  display.setTextSize(1);
  display.setCursor(0, 38);
  display.println(F("   Control Pendant"));
  display.println(F("    Designed By"));
  display.println(F("  Steve Richardson"));
  display.display();      // Show initial text

  delay(2000);
  display.clearDisplay();
  display.display();      // Show initial text
}

void oled_update(char axis, bool axis_updated, char inc, bool inc_updated, bool btn, bool btn_updated, int mpg, bool mpg_updated)
{
  static elapsedMillis elapsed;
  static bool flipper;
  bool timer_updated = false;
  static int last_mpg = -32767;

  if (elapsed >= 500) {
    flipper = !flipper;
    timer_updated = true;
    elapsed = 0;
  }

  if (!axis_updated && !inc_updated && !btn_updated && !mpg_updated && !timer_updated)
    return;
  
  display.clearDisplay();

  display.fillRoundRect(0, 0, 36, 36, 5, SH110X_WHITE);
  
  display.setFont(&FreeSansBold24pt7b);
  display.setTextSize(1);
  display.setTextColor(SH110X_BLACK);
  
  // 'Z' char is a bit narrower in this font, so center it
  if (axis == 'Z') 
    display.setCursor(4, 34);
  else
    display.setCursor(2, 34);
  
  display.println(axis);
  display.setFont();
  display.setTextSize(2);
  display.setTextColor(SH110X_WHITE);

  switch (inc) {
    case 'S':
      display.setCursor(40, 8);
      display.print(F("0.0001\""));
      display.setTextSize(1);
      display.setCursor(40, 24);
      display.print(F("TENTHS"));
      break;
    case 'M':
      display.setCursor(40, 8);
      display.print(F("0.0010\""));
      display.setTextSize(1);
      display.setCursor(40, 24);
      display.print(F("THOUSANDTHS"));
      break;
    case 'L':
      display.setCursor(40, 8);
      display.print(F("0.0100\""));
      display.setTextSize(1);
      display.setCursor(40, 24);
      display.print(F("TEN THOU"));
      break;
    default:
      break;
  }

  display.setTextSize(1);
  display.setFont(&FreeSansBold9pt7b);
  display.setTextColor(SH110X_WHITE);
  switch (axis) {
    case 'X':
      display.setCursor(0, 60);
      display.print("X-");
      display.setCursor(106, 60);
      display.print("X+");
      break;
    case 'Y':
      display.setCursor(0, 60);
      display.print("Y-");
      display.setCursor(106, 60);
      display.print("Y+");
      break;
    case 'Z':
      display.setCursor(0, 60);
      display.print("Z-");
      display.setCursor(106, 60);
      display.print("Z+");
      break;
    default:
      break;
  }

  display.setFont();
  if (btn) {
      display.setCursor(32, 50);
      display.setTextSize(1);
      display.setTextColor(SH110X_WHITE);
      if (!mpg_updated) {
        display.print("JOG ACTIVE");
      } else if (mpg / 4 > last_mpg / 4) {
        display.print("       >>>");
      } else if (mpg / 4 < last_mpg / 4) {
        display.print("<<<");
      }
  } else {
    if (flipper) {
      display.fillRect(28, 48, 72, 12, SH110X_WHITE);
      display.setCursor(32, 50);
      display.setTextSize(1);
      display.setTextColor(SH110X_BLACK);
      display.print("HOLD BUTTON");
    } else {
      display.setCursor(32, 50);
      display.setTextSize(1);
      display.setTextColor(SH110X_WHITE);
      display.print("HOLD BUTTON");
    }
  }
  display.display();

  last_mpg = mpg;
}
