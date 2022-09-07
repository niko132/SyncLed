# SyncLED

An ESP8266/ESP32 based LED controller with focus on usability, customization and synchronizantion. It can controll NeoPixel (WS2812B) LEDs over a modern web interface.

## Features

- Lots of effects
- Fully customizable color palettes
- Multiple effects and palettes through virtual devices
- Save presets and playlists and sync them with other SyncLEDs
- Automatic time synchronization with other SyncLEDs
- Access Point for initial configuration and as a fallback
- 2D effects for LED matrices
- 7 Segment Display options (Time, Countdown, ...)
- Auto Brightness for daytime dependent brightness
- Stream from other software (Hyperion, Audio-reactive-led, ...)

## Parts needed
- ESP8266 / ESP32 microcontroller
- LED strip (WS2812B)
- 5V Power Supply with **enough wattage**
- 5V to 3.3V Regulator (or go the cheap way and just use 2 diodes)

## Hardware Setup
- Connect the LED strip power wires and the Regulator to the Power Supply
- Connect the microcontroller to the Regulator
- Connect the data wire of the LED strip to the TX0 pin of the microcontroller

## Installation
- Download the code as a ZIP
- Extract the files
- Open the .ino file with the Arduino IDE
- Configure the IDE for your microcontroller
- Upload the code through the Arduino IDE
- Upload the additional data with the ***Little FS upload plugin***

After the upload is completed and the microcontroller booted, you should be able to see a new WiFi AP called "**SyncLED #...**". Connect to it and the setup wizzard should appear.