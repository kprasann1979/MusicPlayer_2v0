#include <Arduino.h>
#include <TM1637Display.h>
#include <SoftwareSerial.h>
#include <DFRobotDFPlayerMini.h>

// ======================================================
// PINS
// ======================================================

#define ENC_CLK   3
#define ENC_DT    4
#define ENC_SW    5

#define VOL_UP    8
#define PAUSE_BTN 9
#define VOL_DOWN  10

#define DISP_CLK  6
#define DISP_DIO  7

#define DF_RX 11
#define DF_TX 12

// ======================================================
// HARDWARE
// ======================================================

TM1637Display display(DISP_CLK, DISP_DIO);

SoftwareSerial mp3Serial(DF_RX, DF_TX);
DFRobotDFPlayerMini dfplayer;

// ======================================================
// MODES
// ======================================================

enum Mode
{
    MODE_FOLDER,
    MODE_SONG
};

Mode mode = MODE_FOLDER;

// ======================================================
// STATE
// ======================================================

volatile int encoderDelta = 0;
volatile unsigned long lastISRTime = 0;

int folder = 1;     // 1–12
int song   = 1;     // 1–10 (inside folder)
int volume = 15;

bool isPlaying = true;

bool folderPlaying = false;
int currentTrackInFolder = 1;

// ======================================================
// DISPLAY CONTROL
// ======================================================

enum DisplayState
{
    SHOW_FOLDER,
    SHOW_SONG,
    SHOW_VOLUME,
    SHOW_MESSAGE
};

DisplayState displayState = SHOW_FOLDER;

unsigned long messageUntil = 0;

// ======================================================
// DEBOUNCE
// ======================================================

unsigned long lastVolUpTime = 0;
unsigned long lastVolDownTime = 0;
unsigned long lastPauseTime = 0;

const unsigned long DEBOUNCE = 250;

// ======================================================
// DISPLAY HELPERS
// ======================================================

void showFolder()
{
    uint8_t segs[4];

    // F
    segs[0] = SEG_A | SEG_E | SEG_F | SEG_G;

    // '-'
    segs[1] = SEG_G;

    // tens digit (always shown, even if 0)
    int tens = folder / 10;
    int ones = folder % 10;

    // FORCE leading zero formatting logic
    if (tens == 0)
    {
        segs[2] = display.encodeDigit(0);   // forced '0'
        segs[3] = display.encodeDigit(ones);
    }
    else
    {
        segs[2] = display.encodeDigit(tens);
        segs[3] = display.encodeDigit(ones);
    }

    display.setSegments(segs);
}

void showSong()
{
    display.showNumberDec(song, false);
}

void showVolume()
{
    display.showNumberDec(volume, false);
}

void showPLAY()
{
    const uint8_t seg[] =
    {
        SEG_A | SEG_B | SEG_E | SEG_F | SEG_G,          // P
        SEG_D | SEG_E | SEG_F,                          // L
        SEG_A | SEG_B | SEG_C | SEG_E | SEG_F | SEG_G,  // A
        SEG_B | SEG_C | SEG_D | SEG_F | SEG_G           // Y
    };

    display.setSegments(seg);
}

void showPAUS()
{
    const uint8_t seg[] =
    {
        SEG_A | SEG_B | SEG_E | SEG_F | SEG_G,          // P
        SEG_A | SEG_B | SEG_C | SEG_E | SEG_F | SEG_G,  // A
        SEG_B | SEG_C | SEG_D | SEG_E | SEG_F,          // U
        SEG_A | SEG_C | SEG_D | SEG_F | SEG_G           // S
    };

    display.setSegments(seg);
}

void showSEL()
{
    const uint8_t seg[] =
    {
        SEG_A | SEG_F | SEG_G | SEG_C | SEG_D,          // S
        SEG_A | SEG_D | SEG_E | SEG_F | SEG_G,          // E
        SEG_D | SEG_E | SEG_F,                          // L
        0x00                                            // (blank)
    };

    display.setSegments(seg);
}

// ======================================================
// DFPLAYER ACTIONS
// ======================================================

void playFolder()
{
    currentTrackInFolder = 1;
    folderPlaying = true;

    dfplayer.playFolder(folder, currentTrackInFolder);
}

void playSong()
{
    Serial.print("PLAY SONG: ");
    Serial.print(song);
    Serial.print(" FROM FOLDER ");
    Serial.println(folder);
    folderPlaying = false;
    dfplayer.playFolder(folder, song);
}

// ======================================================
// ENCODER ISR
// ======================================================

void encoderISR()
{
    unsigned long now = micros();

    if (now - lastISRTime < 800)
        return;

    lastISRTime = now;

    bool clk = digitalRead(ENC_CLK);
    bool dt  = digitalRead(ENC_DT);

    if (clk == LOW)
    {
        if (dt != clk)
            encoderDelta++;
        else
            encoderDelta--;
    }
}

// ======================================================
// SETUP
// ======================================================

void setup()
{
    Serial.begin(9600);
    delay(2000); // VERY IMPORTANT
    Serial.println("Starting DFPlayer...");

    pinMode(ENC_CLK, INPUT_PULLUP);
    pinMode(ENC_DT, INPUT_PULLUP);
    pinMode(ENC_SW, INPUT_PULLUP);

    pinMode(VOL_UP, INPUT_PULLUP);
    pinMode(VOL_DOWN, INPUT_PULLUP);
    pinMode(PAUSE_BTN, INPUT_PULLUP);

    display.setBrightness(7);

    mp3Serial.begin(9600);

    if (!dfplayer.begin(mp3Serial))
    {
        Serial.println("DFPLAYER ERROR");
        while (true);
    }

    dfplayer.volume(volume);
    // ==================================================
    // FORCE START STATE (IMPORTANT)
    // ==================================================

    mode = MODE_FOLDER;
    displayState = SHOW_FOLDER;

    folder = 1;
    song = 1;
    showFolder();

    attachInterrupt(digitalPinToInterrupt(ENC_CLK), encoderISR, CHANGE);

    Serial.println("SYSTEM READY");
}

// ======================================================
// LOOP
// ======================================================

void loop()
{
    unsigned long now = millis();

    // ==================================================
    // ENCODER ROTATION
    // ==================================================

    if (encoderDelta != 0)
    {
        noInterrupts();
        int delta = encoderDelta;
        encoderDelta = 0;
        interrupts();

        if (mode == MODE_FOLDER)
        {
            folder += delta;

            if (folder > 12) folder = 1;
            if (folder < 1) folder = 12;

            showFolder();

            Serial.print("FOLDER: ");
            Serial.println(folder);
        }
        else
        {
            song += delta;

            if (song > 10) song = 1;
            if (song < 1) song = 10;

            showSong();

            Serial.print("SONG: ");
            Serial.println(song);
        }
    }

    // ==================================================
    // ENCODER PRESS (short = play, long = toggle mode)
    // ==================================================

    static unsigned long pressStart = 0;
    static bool pressed = false;

    if (digitalRead(ENC_SW) == LOW && !pressed)
    {
        pressed = true;
        pressStart = now;
    }

    if (digitalRead(ENC_SW) == HIGH && pressed)
    {
        pressed = false;

        unsigned long duration = now - pressStart;

        if (duration > 800)
        {
            // LONG PRESS → toggle mode
            if (mode == MODE_FOLDER)
                mode = MODE_SONG;
            else
                mode = MODE_FOLDER;

            showSEL();
            displayState = SHOW_MESSAGE;
            messageUntil = now + 1500;

            Serial.println("MODE TOGGLED");
        }
        else
        {
            // SHORT PRESS
            if (mode == MODE_FOLDER)
                playFolder();
            else
                playSong();
        }
    }

    if (folderPlaying && dfplayer.available())
    {
        uint8_t type = dfplayer.readType();

        if (type == DFPlayerPlayFinished)
        {
            currentTrackInFolder++;

            if (currentTrackInFolder > 10)
            {
                folderPlaying = false;
            }
            else
            {
                dfplayer.playFolder(folder, currentTrackInFolder);
            }
        }
    }    

    // ==================================================
    // VOLUME UP
    // ==================================================

    if (digitalRead(VOL_UP) == LOW &&
        now - lastVolUpTime > DEBOUNCE)
    {
        if (volume < 30) volume++;

        dfplayer.volume(volume);

        showVolume();
        displayState = SHOW_VOLUME;
        messageUntil = now + 1000;

        Serial.print("VOL: ");
        Serial.println(volume);

        lastVolUpTime = now;
    }

    // ==================================================
    // VOLUME DOWN
    // ==================================================

    if (digitalRead(VOL_DOWN) == LOW &&
        now - lastVolDownTime > DEBOUNCE)
    {
        if (volume > 0) volume--;

        dfplayer.volume(volume);

        showVolume();
        displayState = SHOW_VOLUME;
        messageUntil = now + 1000;

        Serial.print("VOL: ");
        Serial.println(volume);

        lastVolDownTime = now;
    }

    // ==================================================
    // PAUSE BUTTON
    // ==================================================

    if (digitalRead(PAUSE_BTN) == LOW &&
        now - lastPauseTime > DEBOUNCE)
    {
        isPlaying = !isPlaying;

        if (isPlaying)
        {
            dfplayer.start();     // Resume playback
            showPLAY();
        }
        else
        {
            dfplayer.pause();     // Pause playback
            showPAUS();
        }

        displayState = SHOW_MESSAGE;
        messageUntil = now + 3000;

        lastPauseTime = now;
    }

    // ==================================================
    // RESTORE DISPLAY
    // ==================================================

    if (displayState != SHOW_FOLDER &&
        displayState != SHOW_SONG &&
        millis() > messageUntil)
    {
        if (mode == MODE_FOLDER)
            showFolder();
        else
            showSong();

        displayState = (mode == MODE_FOLDER) ? SHOW_FOLDER : SHOW_SONG;
    }
}