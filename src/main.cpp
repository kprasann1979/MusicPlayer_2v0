#include <SoftwareSerial.h>
#include <DFRobotDFPlayerMini.h>
#include <TM1637Display.h>

// ======================================================
// CONFIG
// ======================================================

#define TEST_MODE false  // Set to true to test without MP3 module

// ======================================================
// PINS
// ======================================================

// Rotary Encoder
#define ENC_CLK 3      // Interrupt pin
#define ENC_DT  4
#define ENC_SW  5

// Buttons
#define PAUSE_BTN 9
#define VOL_UP    8
#define VOL_DOWN  10

// DFPlayer
#define DF_RX 11
#define DF_TX 12

// TM1637
#define DISP_CLK 6
#define DISP_DIO 7

// ======================================================
// OBJECTS
// ======================================================

SoftwareSerial dfSerial(DF_RX, DF_TX);
DFRobotDFPlayerMini dfPlayer;
TM1637Display display(DISP_CLK, DISP_DIO);

// ======================================================
// VARIABLES
// ======================================================

volatile int8_t encoderDelta = 0;
volatile unsigned long lastISRTime = 0;

uint8_t songNumber = 1;
uint8_t volumeLevel = 10;

// States
bool isPlaying = false;
bool isPaused  = false;

// Display timeout
bool showingStatus = false;
unsigned long statusDisplayTime = 0;

// Button states
bool lastPlayBtnState   = HIGH;
bool lastPauseBtnState  = HIGH;
bool lastVolUpState     = HIGH;
bool lastVolDownState   = HIGH;

// Encoder speed
unsigned long lastMoveTime = 0;

// ======================================================
// DISPLAY HELPERS
// ======================================================

void showNumber() {

  display.showNumberDec(songNumber, false);

  if (TEST_MODE) {
    Serial.print("Song: ");
    Serial.println(songNumber);
  }
}

void displayPLAY() {

  uint8_t data[] = {
    0x73, // P
    0x38, // L
    0x77, // A
    0x6E  // Y
  };

  display.setSegments(data);

  if (TEST_MODE) {
    Serial.println("DISPLAY: PLAY");
  }
}

void displayPAUS() {

  uint8_t data[] = {
    0x73, // P
    0x77, // A
    0x3E, // U
    0x6D  // S
  };

  display.setSegments(data);

  if (TEST_MODE) {
    Serial.println("DISPLAY: PAUS");
  }
}

void displayVOL() {

  display.showNumberDec(volumeLevel, false);

  if (TEST_MODE) {
    Serial.print("Volume: ");
    Serial.println(volumeLevel);
  }
}

// ======================================================
// ROTARY ENCODER ISR
// ======================================================

void readEncoderISR() {

  unsigned long now = micros();

  // Debounce
  if (now - lastISRTime < 1200) {
    return;
  }

  lastISRTime = now;

  // Direction
  if (digitalRead(ENC_CLK) == digitalRead(ENC_DT)) {
    encoderDelta--;
  }
  else {
    encoderDelta++;
  }
}

// ======================================================
// HANDLE ENCODER
// ======================================================

void handleEncoder() {

  int8_t delta = 0;

  noInterrupts();
  delta = encoderDelta;
  encoderDelta = 0;
  interrupts();

  if (delta == 0) {
    return;
  }

  unsigned long now = millis();
  unsigned long speed = now - lastMoveTime;
  lastMoveTime = now;

  int stepSize;

  // Speed acceleration
  if (speed < 30) {
    stepSize = 10;
  }
  else if (speed < 80) {
    stepSize = 3;
  }
  else {
    stepSize = 1;
  }

  int newSong = songNumber + (delta * stepSize);

  if (newSong > 99) newSong = 1;
  if (newSong < 1)  newSong = 99;

  songNumber = newSong;

  if (!showingStatus && !isPaused) {
    showNumber();
  }

  if (TEST_MODE) {
    Serial.print("Selected Song: ");
    Serial.println(songNumber);
  }
}

// ======================================================
// PLAY BUTTON
// ======================================================

void handlePlayButton() {

  bool currentState = digitalRead(ENC_SW);

  if (lastPlayBtnState == HIGH &&
      currentState == LOW) {

    delay(20);

    if (digitalRead(ENC_SW) == LOW) {

      if (!TEST_MODE) {

        dfPlayer.stop();
        delay(100);

        dfPlayer.play(songNumber);
      }

      displayPLAY();

      isPlaying = true;
      isPaused  = false;

      showingStatus = true;
      statusDisplayTime = millis();

      if (TEST_MODE) {
        Serial.print("PLAY SONG: ");
        Serial.println(songNumber);
      }
    }
  }

  lastPlayBtnState = currentState;
}

// ======================================================
// PAUSE BUTTON
// ======================================================

void handlePauseButton() {

  bool currentState = digitalRead(PAUSE_BTN);

  if (lastPauseBtnState == HIGH &&
      currentState == LOW) {

    delay(20);

    if (digitalRead(PAUSE_BTN) == LOW &&
        isPlaying) {

      if (!isPaused) {

        if (!TEST_MODE) {
          dfPlayer.pause();
        }

        displayPAUS();

        isPaused = true;

        if (TEST_MODE) {
          Serial.println("PAUSED");
        }
      }
      else {

        if (!TEST_MODE) {
          dfPlayer.start();
        }

        displayPLAY();

        isPaused = false;

        if (TEST_MODE) {
          Serial.println("RESUMED");
        }
      }

      showingStatus = true;
      statusDisplayTime = millis();
    }
  }

  lastPauseBtnState = currentState;
}

// ======================================================
// VOLUME UP
// ======================================================

void handleVolumeUp() {

  bool currentState = digitalRead(VOL_UP);

  if (lastVolUpState == HIGH &&
      currentState == LOW) {

    delay(20);

    if (digitalRead(VOL_UP) == LOW) {

      if (volumeLevel < 30) {
        volumeLevel++;
      }

      if (!TEST_MODE) {
        dfPlayer.volume(volumeLevel);
      }

      displayVOL();

      showingStatus = true;
      statusDisplayTime = millis();

      if (TEST_MODE) {
        Serial.print("VOL UP: ");
        Serial.println(volumeLevel);
      }
    }
  }

  lastVolUpState = currentState;
}

// ======================================================
// VOLUME DOWN
// ======================================================

void handleVolumeDown() {

  bool currentState = digitalRead(VOL_DOWN);

  if (lastVolDownState == HIGH &&
      currentState == LOW) {

    delay(20);

    if (digitalRead(VOL_DOWN) == LOW) {

      if (volumeLevel > 0) {
        volumeLevel--;
      }

      if (!TEST_MODE) {
        dfPlayer.volume(volumeLevel);
      }

      displayVOL();

      showingStatus = true;
      statusDisplayTime = millis();

      if (TEST_MODE) {
        Serial.print("VOL DOWN: ");
        Serial.println(volumeLevel);
      }
    }
  }

  lastVolDownState = currentState;
}

// ======================================================
// DISPLAY TIMEOUT
// ======================================================

void handleDisplayTimeout() {

  // Keep PAUS permanently while paused
  if (isPaused) {
    return;
  }

  if (showingStatus &&
      millis() - statusDisplayTime >= 3000) {

    showingStatus = false;

    showNumber();

    if (TEST_MODE) {
      Serial.println("Back to Song Display");
    }
  }
}

// ======================================================
// DFPLAYER INIT
// ======================================================

void initDFPlayer() {

  dfSerial.begin(9600);

  if (dfPlayer.begin(dfSerial)) {

    dfPlayer.volume(volumeLevel);

    Serial.println("DFPlayer Ready");
  }
  else {

    Serial.println("DFPlayer NOT detected");
  }
}

// ======================================================
// SETUP
// ======================================================

void setup() {

  Serial.begin(115200);

  // Inputs
  pinMode(ENC_CLK, INPUT_PULLUP);
  pinMode(ENC_DT, INPUT_PULLUP);
  pinMode(ENC_SW, INPUT_PULLUP);

  pinMode(PAUSE_BTN, INPUT_PULLUP);
  pinMode(VOL_UP, INPUT_PULLUP);
  pinMode(VOL_DOWN, INPUT_PULLUP);

  // Display
  display.setBrightness(3);

  showNumber();

  // Encoder interrupt
  attachInterrupt(
    digitalPinToInterrupt(ENC_CLK),
    readEncoderISR,
    FALLING
  );

  // DFPlayer
  if (!TEST_MODE) {
    initDFPlayer();
  }

  Serial.println();
  Serial.println("======================");
  Serial.println("SYSTEM READY");
  Serial.println("======================");
}

// ======================================================
// MAIN LOOP
// ======================================================

void loop() {

  handleEncoder();

  handlePlayButton();

  handlePauseButton();

  handleVolumeUp();

  handleVolumeDown();

  handleDisplayTimeout();
}