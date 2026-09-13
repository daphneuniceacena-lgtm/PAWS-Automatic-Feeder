/*
  PAWS FEEDER - Automatic Feeder
  ---------------------------------------------------
  - Button 1 (D5): Set HOUR   -> 1 click = +1 hr, DOUBLE click = Confirm/Save
  - Button 2 (D6): Set MINUTE -> 1 click = +1 min, DOUBLE click = Confirm/Save
  - Button 3 (D2): Manual Feed -> Immediate dispense
  - 16x2 I2C LCD with centered text
  - Real-time countdown on default screen (HH:MM:SS)
  - SG90 Servo: 105° (Tilted upward at rest) to 180° (Dispensing)
  - DS1302 RTC module for timekeeping

  WIRING RECAP:
  - Button 1 (Hour): D5 to GND (Diagonally opposite legs)
  - Button 2 (Minute): D6 to GND (Diagonally opposite legs)
  - Button 3 (Manual Feed): D2 to GND (Diagonally opposite legs)
  - NO 5V WIRES CONNECTED TO ANY BUTTONS!
  - LCD I2C: SDA -> A4, SCL -> A5
  - RTC: DAT -> D7, CLK -> D8, RST -> D9
  - Servo: Signal -> D4
*/

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <ThreeWire.h>
#include <RtcDS1302.h>
#include <Servo.h>

#define SERVO_PIN   4
#define BUTTON1_PIN 5   // Hour set (D5)
#define BUTTON2_PIN 6   // Minute set (D6)
#define BUTTON3_PIN 2   // Manual feed (D2)

LiquidCrystal_I2C lcd(0x27, 16, 2);
const int LCD_COLS = 16;

ThreeWire myWire(7, 8, 9); // DAT = D7, CLK = D8, RST = D9
RtcDS1302<ThreeWire> Rtc(myWire);

Servo feederServo;
const int SERVO_CLOSED_POS = 105;  // Tilted upward at rest
const int SERVO_OPEN_POS   = 180;  // Open position dispensing
const int FEED_OPEN_TIME_MS = 800; 

const uint32_t FEED_INTERVAL_SECONDS = 10800; // 3 hours
uint32_t lastFeedEpoch = 0;

enum ScreenState { SCREEN_DEFAULT, SCREEN_SET_HOUR, SCREEN_SET_MINUTE, SCREEN_DISPENSING };
ScreenState screen = SCREEN_DEFAULT;

int setHour = 0;
int setMinute = 0;

unsigned long lastLcdUpdate = 0;
const unsigned long LCD_UPDATE_MS = 200;

unsigned long lastBtn1Click = 0;
unsigned long lastBtn2Click = 0;
const unsigned long DOUBLE_CLICK_WINDOW = 400;

void setup() {
  Serial.begin(9600);

  pinMode(BUTTON1_PIN, INPUT_PULLUP);
  pinMode(BUTTON2_PIN, INPUT_PULLUP);
  pinMode(BUTTON3_PIN, INPUT_PULLUP);

  Wire.begin();
  lcd.init();
  lcd.backlight();
  printCentered(0, "PAWS feeder");
  printCentered(1, "Starting...");

  Rtc.Begin();
  if (!Rtc.IsDateTimeValid()) {
    RtcDateTime defaultTime = RtcDateTime(__DATE__, __TIME__);
    Rtc.SetDateTime(defaultTime);
  }
  if (Rtc.GetIsWriteProtected()) Rtc.SetIsWriteProtected(false);
  if (!Rtc.GetIsRunning()) Rtc.SetIsRunning(true);

  RtcDateTime now = Rtc.GetDateTime();
  lastFeedEpoch = now.Epoch32Time();

  feederServo.attach(SERVO_PIN);
  feederServo.write(SERVO_CLOSED_POS);
  delay(500);
  feederServo.detach();

  delay(1000);
  lcd.clear();
}

void loop() {
  handleButtons();
  checkAutoFeed();

  if (millis() - lastLcdUpdate >= LCD_UPDATE_MS) {
    lastLcdUpdate = millis();
    redrawScreen();
  }
}

void handleButtons() {
  static bool prevB1 = HIGH, prevB2 = HIGH, prevB3 = HIGH;
  bool currB1 = digitalRead(BUTTON1_PIN);
  bool currB2 = digitalRead(BUTTON2_PIN);
  bool currB3 = digitalRead(BUTTON3_PIN);

  // --- Button 1 (Hour Set - D5) ---
  if (prevB1 == HIGH && currB1 == LOW) {
    delay(40); // Debounce
    unsigned long now = millis();
    if (screen == SCREEN_SET_HOUR && (now - lastBtn1Click < DOUBLE_CLICK_WINDOW)) {
      saveSetTimeToRtc();
      screen = SCREEN_DEFAULT;
    } else {
      if (screen != SCREEN_SET_HOUR) {
        enterSetMode(SCREEN_SET_HOUR);
      } else {
        setHour = (setHour + 1) % 24;
      }
      lastBtn1Click = now;
    }
  }

  if (prevB2 == HIGH && currB2 == LOW) {
    delay(40); // Debounce
    unsigned long now = millis();
    if (screen == SCREEN_SET_MINUTE && (now - lastBtn2Click < DOUBLE_CLICK_WINDOW)) {
      saveSetTimeToRtc();
      screen = SCREEN_DEFAULT;
    } else {
      if (screen != SCREEN_SET_MINUTE) {
        enterSetMode(SCREEN_SET_MINUTE);
      } else {
        setMinute = (setMinute + 1) % 60;
      }
      lastBtn2Click = now;
    }
  }


  if (prevB3 == HIGH && currB3 == LOW) {
    delay(40); // Debounce
    if (screen == SCREEN_DEFAULT) {
      feedNow();
    }
  }

  prevB1 = currB1;
  prevB2 = currB2;
  prevB3 = currB3;
}

void enterSetMode(ScreenState which) {
  RtcDateTime now = Rtc.GetDateTime();
  setHour = now.Hour();
  setMinute = now.Minute();
  screen = which;
}

void saveSetTimeToRtc() {
  RtcDateTime current = Rtc.GetDateTime();
  RtcDateTime updated(current.Year(), current.Month(), current.Day(), setHour, setMinute, 0);
  Rtc.SetDateTime(updated);
  lastFeedEpoch = updated.Epoch32Time(); // Reset 3-hour timer benchmark
}


void checkAutoFeed() {
  if (screen != SCREEN_DEFAULT) return;

  RtcDateTime now = Rtc.GetDateTime();
  uint32_t currentEpoch = now.Epoch32Time();

  if (currentEpoch - lastFeedEpoch >= FEED_INTERVAL_SECONDS) {
    feedNow();
  }
}

void feedNow() {
  screen = SCREEN_DISPENSING;
  redrawScreen();

  feederServo.attach(SERVO_PIN);
  feederServo.write(SERVO_OPEN_POS);
  delay(FEED_OPEN_TIME_MS);
  feederServo.write(SERVO_CLOSED_POS);
  delay(500);
  feederServo.detach();

  RtcDateTime now = Rtc.GetDateTime();
  lastFeedEpoch = now.Epoch32Time();

  delay(1200);
  screen = SCREEN_DEFAULT;
}

void redrawScreen() {
  switch (screen) {
    case SCREEN_DEFAULT:    drawDefaultScreen();   break;
    case SCREEN_SET_HOUR:   drawSetScreen("Set time (Hour)");   break;
    case SCREEN_SET_MINUTE: drawSetScreen("Set time (Minute)"); break;
    case SCREEN_DISPENSING: drawDispensingScreen(); break;
  }
}

void drawDefaultScreen() {
  RtcDateTime now = Rtc.GetDateTime();
  uint32_t currentEpoch = now.Epoch32Time();
  
  uint32_t remainingSeconds = 0;
  if (currentEpoch - lastFeedEpoch < FEED_INTERVAL_SECONDS) {
    remainingSeconds = FEED_INTERVAL_SECONDS - (currentEpoch - lastFeedEpoch);
  }

  int hrs = remainingSeconds / 3600;
  int mins = (remainingSeconds % 3600) / 60;
  int secs = remainingSeconds % 60;

  char line2[17];
  snprintf(line2, sizeof(line2), "Next: %02d:%02d:%02d", hrs, mins, secs);

  printCentered(0, "PAWS feeder");
  printCentered(1, line2);
}

void drawSetScreen(const char* title) {
  char line2[17];
  snprintf(line2, sizeof(line2), "%02d:%02d", setHour, setMinute);

  printCentered(0, title);
  printCentered(1, line2);
}

void drawDispensingScreen() {
  printCentered(0, "Dispensing now...");
  printCentered(1, "");
}

void printCentered(int row, const char* text) {
  int len = strlen(text);
  if (len > LCD_COLS) len = LCD_COLS;
  int padLeft = (LCD_COLS - len) / 2;

  lcd.setCursor(0, row);
  for (int i = 0; i < LCD_COLS; i++) lcd.print(' ');

  lcd.setCursor(padLeft, row);
  lcd.print(text);
}