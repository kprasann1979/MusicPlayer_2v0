#include <SoftwareSerial.h>
#include <DFRobotDFPlayerMini.h>
#include <TM1637Display.h>

// ===== CONFIG =====
#define TEST_MODE false  // Set to true to test without MP3 module

// ===== Pins =====
#define ENC_CLK 2      // MUST be interrupt pin
#define ENC_DT  3
#define ENC_SW  4
#define PAUSE_BTN 5
#define BUZZER 8

#define DF_RX 10
#define DF_TX 11

#define DISP_CLK 6
#define DISP_DIO 7

SoftwareSerial dfSerial(DF_RX, DF_TX);
DFRobotDFPlayerMini dfPlayer;
TM1637Display display(DISP_CLK, DISP_DIO);

// ===== Variables =====
volatile int songNumber = 1;
volatile bool encoderMoved = false;
volatile unsigned long lastEncoderTime = 0;
volatile int stepSize = 1;

bool isPlaying = false;
bool isPaused  = false;

unsigned long statusDisplayTime = 0;
bool showingStatus = false;

// ===== BEEP =====
void beep() {
  tone(BUZZER, 2000, 100);
}

// ===== INTERRUPT ENCODER =====
void readEncoderISR() {

  unsigned long now = millis();
  unsigned long delta = now - lastEncoderTime;
  lastEncoderTime = now;

  // Determine step size based on speed
  if (delta < 30)
    stepSize = 10;     // Fast spin
  else if (delta < 80)
    stepSize = 3;      // Medium spin
  else
    stepSize = 1;      // Slow spin

  if (digitalRead(ENC_DT) == LOW)
    songNumber += stepSize;
  else
    songNumber -= stepSize;

  // Wrap 1–99
  if (songNumber > 99) songNumber = 99;
  if (songNumber < 1)  songNumber = 1;

  encoderMoved = true;

  if (TEST_MODE) {
    Serial.print("Song: ");
    Serial.print(songNumber);
    Serial.print("  Step: ");
    Serial.println(stepSize);
  }
}
// ===== SHOW NUMBER =====
void showNumber() {
  display.showNumberDecEx(songNumber, 0, false);
  if (TEST_MODE) {
    Serial.print("Displaying number: ");
    Serial.println(songNumber);
  }
}

// ===== DISPLAY PLAY =====
void displayPLAY() {
  uint8_t data[] = {0x73, 0x38, 0x77, 0x6E}; // P L A Y
  display.setSegments(data);
  if (TEST_MODE) Serial.println("Displaying: PLAY");
}

// ===== DISPLAY PAUS =====
void displayPAUS() {
  uint8_t data[] = {0x73, 0x77, 0x3E, 0x6D}; // P A U S
  display.setSegments(data);
  if (TEST_MODE) Serial.println("Displaying: PAUS");
}

// ===== PLAY BUTTON =====
void readPlayButton() {

  if (digitalRead(ENC_SW) == LOW) {

    delay(200);  // debounce

    beep();

    if (!TEST_MODE) {
      dfPlayer.stop();
      delay(100);
      dfPlayer.play(songNumber);
    }

    displayPLAY();

    isPlaying = true;   // Always playing after this
    isPaused  = false;

    statusDisplayTime = millis();
    showingStatus = true;

    if (TEST_MODE) {
      Serial.print("PLAY pressed → Song: ");
      Serial.println(songNumber);
    }

    // Wait until button released (prevents repeat trigger)
    while (digitalRead(ENC_SW) == LOW);
  }
}


// ===== PAUSE BUTTON =====
void readPauseButton() {
  if (digitalRead(PAUSE_BTN) == LOW && isPlaying) {
    delay(200);

    if (!isPaused) {
      if (!TEST_MODE) dfPlayer.pause();
      displayPAUS();
      isPaused = true;
      if (TEST_MODE) Serial.println("PAUSED");
    } else {
      if (!TEST_MODE) dfPlayer.start();
      displayPLAY();
      isPaused = false;
      if (TEST_MODE) Serial.println("RESUMED");
    }

    statusDisplayTime = millis();
    showingStatus = true;
  }
}

// ===== AUTO RETURN AFTER 5 SEC =====
void autoReturnDisplay() {

  // If paused, keep showing PAUS forever
  if (isPaused) return;

  // Only return after PLAY
  if (isPlaying && showingStatus && millis() - statusDisplayTime > 5000) {
    showNumber();
    showingStatus = false;

    if (TEST_MODE) {
      Serial.println("Back to number display");
    }
  }
}

// ===== SETUP =====
void setup() {
  Serial.begin(9600);

  pinMode(ENC_CLK, INPUT_PULLUP);
  pinMode(ENC_DT, INPUT_PULLUP);
  pinMode(ENC_SW, INPUT_PULLUP);
  pinMode(PAUSE_BTN, INPUT_PULLUP);
  pinMode(BUZZER, OUTPUT);

  attachInterrupt(digitalPinToInterrupt(ENC_CLK), readEncoderISR, FALLING);

  display.setBrightness(1);
  showNumber();

  if (!TEST_MODE) {
    dfSerial.begin(9600);
    if (dfPlayer.begin(dfSerial)) {
      dfPlayer.volume(20);
    }
  }

  Serial.println("===== TEST MODE STARTED =====");
}

// ===== LOOP =====
void loop() {
  readPlayButton();
  readPauseButton();

  if (encoderMoved && !isPaused) {
    encoderMoved = false;
    showNumber();
  }

  autoReturnDisplay();
}
