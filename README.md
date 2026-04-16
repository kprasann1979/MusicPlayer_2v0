# 🎵 Arduino Music Box v2.0 - Rotary Encoder Edition

An upgraded Arduino music box featuring a rotary encoder for smooth song selection, a 4-digit display to show the current track, and play/pause functionality. This version offers a more modern and intuitive user interface compared to the basic switch-based version.

## 📋 What This Project Does

This advanced music box allows you to:
- **Select songs 1-99** using a smooth rotary encoder (like a volume knob)
- **See the current track number** on a 4-digit 7-segment display
- **Play and pause music** with dedicated buttons
- **Fast-track selection** - spin faster to jump by 10s, slower for 1s
- **Visual feedback** with "PLAY" and "PAUS" display modes
- **Audible feedback** with a beep when buttons are pressed

## 🛠️ What You'll Need

### Hardware Components
| Component | Quantity | Purpose |
|-----------|----------|---------|
| Arduino Nano (ATmega328P) | 1 | The brain of the project |
| DFPlayer Mini MP3 Module | 1 | Plays MP3 files |
| MicroSD Card (with MP3 files) | 1 | Stores your music |
| Rotary Encoder (with push button) | 1 | Smooth song selection and play button |
| Push Button | 1 | Pause/resume control |
| 4-Digit 7-Segment Display (TM1637) | 1 | Shows track numbers and status |
| Buzzer (8Ω) | 1 | Audible button feedback |
| Jumper Wires | Many | Connect everything |
| Breadboard | 1 | For prototyping |
| Speaker (3W, 4Ω) | 1 | For audio output |

### Software
- **PlatformIO** (or Arduino IDE)
- **Arduino Core** for ATmega328P
- **Libraries** (automatically installed by PlatformIO):
  - `DFRobotDFPlayerMini` - Controls the MP3 player
  - `SoftwareSerial` - Creates a serial connection for the DFPlayer
  - `TM1637 Driver` - Controls the 4-digit display

## 🔌 Wiring Diagram

### Arduino Nano Pin Connections

| Component | Arduino Pin | Notes |
|-----------|-------------|-------|
| Rotary Encoder CLK | Digital Pin 2 | **Must be interrupt pin** |
| Rotary Encoder DT | Digital Pin 3 | Data pin for rotation |
| Rotary Encoder SW | Digital Pin 4 | Push button (play) |
| Pause Button | Digital Pin 5 | Pause/resume control |
| Buzzer | Digital Pin 8 | Audio feedback |
| DFPlayer TX | Digital Pin 10 | Software Serial RX |
| DFPlayer RX | Digital Pin 11 | Software Serial TX |
| Display CLK | Digital Pin 6 | TM1637 clock |
| Display DIO | Digital Pin 7 | TM1637 data |
| DFPlayer VCC | 5V | Power |
| DFPlayer GND | GND | Ground |
| Display VCC | 5V | Power |
| Display GND | GND | Ground |
| DFPlayer SPK1/SPK2 | Speaker | Audio output |

### Rotary Encoder Wiring
- CLK → Pin 2
- DT → Pin 3  
- SW → Pin 4
- + → 5V
- GND → GND

### TM1637 Display Wiring
- CLK → Pin 6
- DIO → Pin 7
- VCC → 5V
- GND → GND

### Push Button Wiring
- One terminal → Pin 5
- Other terminal → GND
- (Internal pull-up resistor is used)

### Buzzer Wiring
- Positive → Pin 8
- Negative → GND

## 📁 Setting Up the SD Card

1. **Format your MicroSD card** as FAT32
2. **Create a folder** named `mp3` in the root directory
3. **Name your MP3 files** with 3-digit numbers:
   - `001.mp3` - Song 1
   - `002.mp3` - Song 2
   - ... up to `099.mp3` - Song 99
4. **Insert the SD card** into the DFPlayer module

## 🚀 How to Use

### Building and Uploading
1. **Connect your Arduino Nano** to your computer via USB
2. **Open the project** in PlatformIO (or Arduino IDE)
3. **Click "Build and Upload"** (or the upload button)
4. **Wait for completion** - you should see "TEST MODE STARTED" in the serial monitor

### Operating the Music Box
1. **Turn the rotary encoder** to select a song (1-99)
   - **Slow turn**: Changes by 1 (1, 2, 3...)
   - **Fast turn**: Changes by 10 (10, 20, 30...)
2. **Press the rotary encoder** to play the selected song
   - The display will show "PLAY" and the song will start
   - After 5 seconds, it returns to showing the track number
3. **Press the pause button** to pause/resume
   - Display shows "PAUS" when paused
   - Display shows "PLAY" when resumed

## 💡 How It Works

### Key Features Explained

#### Rotary Encoder Magic
- Uses **interrupts** for smooth, responsive control
- **Speed-sensitive stepping**: Fast spins jump by 10, slow spins by 1
- **Automatic wrapping**: Goes from 99 back to 1, and 1 to 99

#### Smart Display
- Shows track numbers (1-99) normally
- Displays "PLAY" when a song starts (for 5 seconds)
- Displays "PAUS" when paused (stays until resumed)

#### Test Mode
- Set `TEST_MODE = true` in the code to test without the DFPlayer
- Shows all actions in the serial monitor
- Perfect for debugging wiring or testing logic

### Code Highlights
```cpp
// Interrupt-driven encoder reading
attachInterrupt(digitalPinToInterrupt(ENC_CLK), readEncoderISR, FALLING);

// Speed-sensitive stepping
if (delta < 30) stepSize = 10;    // Fast spin
else if (delta < 80) stepSize = 3; // Medium spin  
else stepSize = 1;                 // Slow spin
```

## 🔧 Troubleshooting

### Display Not Working
- Check CLK and DIO pins are connected correctly
- Ensure the display has power (5V and GND)
- Verify the TM1637 library is installed

### Encoder Not Responding
- **Critical**: CLK pin MUST be connected to pin 2 (interrupt pin)
- Check all encoder connections (CLK, DT, SW, +, GND)
- Test with a multimeter to ensure proper grounding

### No Sound
- Check speaker connections (SPK1 and SPK2)
- Ensure MP3 files are correctly named (001.mp3 to 099.mp3)
- Verify volume is set (code sets it to 20 out of 30)
- Check if the speaker is working

### Songs Not Playing
- Ensure the SD card is properly formatted and inserted
- Check that MP3 files are in the `mp3` folder
- Verify the DFPlayer has power and is connected correctly

### Buzzer Not Beeping
- Check buzzer polarity (positive to pin 8, negative to GND)
- Ensure the buzzer is the correct type (8Ω)

## 📝 Notes for Beginners

### What is a Rotary Encoder?
A rotary encoder is like a digital volume knob. It can detect both rotation direction and speed, making it perfect for smooth menu navigation.

### What is TM1637?
The TM1637 is a driver chip that controls 4-digit 7-segment displays. It uses only 2 wires (clock and data) to communicate with the Arduino.

### What are Interrupts?
Interrupts allow the Arduino to respond immediately to events (like encoder rotation) without constantly checking them in the main loop. This makes the interface much more responsive.

### Test Mode Benefits
The test mode is incredibly useful for:
- Testing your wiring without needing the DFPlayer
- Debugging issues by watching serial output
- Understanding how the encoder speed affects stepping

## 🎯 Possible Enhancements

Once you've mastered this project, try these modifications:
- Add a volume control potentiometer
- Implement a playlist mode that plays multiple songs
- Add RGB LEDs that change color with the music
- Create a "favorites" button that remembers your top songs
- Add a sleep mode that turns off the display after inactivity
- Implement shuffle/random play functionality

## 📄 License

This project is open source and free to use for personal and educational purposes.

## 🙏 Credits

- **DFRobot** for the DFPlayer Mini library
- **akj7** for the TM1637 Driver library
- **Arduino community** for endless inspiration
- **You** for building this awesome project!

---

**Happy Coding! 🚀**

*If you have any questions or suggestions, feel free to open an issue or contribute to this project.*