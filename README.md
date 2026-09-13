# PAWS-Automatic-Feeder
This is the official repository for an Automatic Feeder for cats, dogs, etc. This is meant for Arduino, DS1302 RTC module, Servo SG90, and LCD 13x2 I2C.
Daphne Eunice U. Acena | daphneuniceacena@gmail.com 

# Technical Documentation and Hardware Interface Specification

The PAWS Feeder is a microcontroller-based automatic pet feeding system built using C/C++ for
Arduino. The system provides scheduled automated feeding intervals, manual dispense capabilities,
real-time countdown tracking on a 16x2 I2C LCD display, and an interactive time-setting interface
managed via hardware push buttons.

# Technical Specification;

Features:

 Automated Feeding Cycle: Dispenses food automatically every 3 hours (10,800 seconds).
 Real-Time Countdown: Continuously updates a dynamic 16x2 LCD displaying remaining time in
HH:MM:SS format.
 Manual Dispense Capability: Dedicated manual feed button triggers an immediate dispense cycle
without resetting overall schedule sync.
 RTC Real-Time Tracking: Utilizes a DS1302 Real-Time Clock module for robust schedule
persistence across board resets.
 Servo Actuation Angles: SG90 servo operates between 105° (resting upward tilt) and 180° (open
dispensing position) with an 800 ms dwell time.
 Power Optimization: Servo is dynamically attached during motor movement and detached post-
actuation to eliminate continuous power draw and mechanical jitter.
 Interactive Control Logic: Debounced push-button interface supporting single-click increments and
double-click commit operations.

# Hardware Components

 Microcontroller: Arduino (UNO, Nano, or compatible ATmega328P development board)
 Real-Time Clock: DS1302 RTC Module
 Display Unit: 16x2 Character LCD with I2C Interface Board (Default Address: 0x27)
 Actuator: SG90 Micro Servo Motor
 User Inputs: 3x Momentary Tactile Push Buttons
 Prototyping Gear: Solderless Breadboard, Jumper Wires, 5V Regulated Power Source

# Wiring

Pin Configuration
CRITICAL WIRING NOTICE: All push buttons utilize internal pull-up resistors (INPUT_PULLUP). Connect
one side of each button to its assigned digital pin and the diagonally opposite leg to GND. DO NOT
connect 5V or external power to button pins.

Push Button Pinouts
Function / Component Arduino Pin Connection Diagram

Button 1 (Hour Set) D5 Pin D5 to GND (Internal Pull-Up)
Button 2 (Minute Set) D6 Pin D6 to GND (Internal Pull-Up)
Button 3 (Manual Feed) D2 Pin D2 to GND (Internal Pull-Up)

Peripheral Pin Mapping
Peripheral Module Module Pin Arduino Connection
16x2 I2C LCD VCC 5V Rail
16x2 I2C LCD GND GND Rail
16x2 I2C LCD SDA Analog Pin A4
16x2 I2C LCD SCL Analog Pin A5
DS1302 RTC DAT (Data) Digital Pin D7
DS1302 RTC CLK (Clock) Digital Pin D8
DS1302 RTC RST (Reset) Digital Pin D9
SG90 Servo Motor Signal (Yellow/Orange) Digital Pin D4
SG90 Servo Motor Power (Red / Black) 5V / GND Rail

# Software Dependencies

 Wire.h: Standard Arduino library handling I2C communication with the LCD module.
 LiquidCrystal_I2C.h: Controls alphanumeric character output and backlight commands for I2C
LCDs.
 ThreeWire.h;RtcDS1302.h: Provides low-level bus interaction and high-level date/time methods
for the DS1302 RTC.
 Servo.h: Generates PWM signals required for standard micro servo positioning.
5. User Interface; Operational Modes

# Default Monitoring Screen

 Line 1: Displays centered program header (PAWS feeder).
 Line 2: Displays dynamic 1-second countdown till next feed (Next: HH:MM:SS;).
Time Adjustment Modes
 Hour Adjustment Mode: Press Button 1 (D5) once to enter. Single clicks advance set hour (0-23).
Double-click Button 1 within 400 ms to confirm and save settings.
 Minute Adjustment Mode: Press Button 2 (D6) once to enter. Single clicks advance set minute (0-
59). Double-click Button 2 within 400 ms to confirm and save settings.
Manual Feed Command
 Immediate Actuation: Pressing Button 3 (D2) from the default screen triggers an immediate feed
cycle and resets the 3-hour timer benchmark epoch.
