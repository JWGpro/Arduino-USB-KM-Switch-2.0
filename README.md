# Arduino-USB-KM-Switch-2.0
Arduino KM (Keyboard + Mouse) Switch version 2.0 is now using USB Host Shield with MAX3421E, using Arduino DUE for better performance, it has 4 Hardware Serial Bus connected with Arduino Micro (Leonardo ATMEGA32U4). The configuration is set to support 4 HID devices in 1 hub (but I recommend only one Keyboard and Mouse, not for example: 2 Keyboards with 2 Mouses, but in many cases it will work). Main software Library is UHS30 (Pre-release of USB Host Library version 3.0). It is using HID RAW readings (and if you have a mouse not working, the solution is to map raw data from the mouse in source code, because I tested only 4 mouses and each has different RAW data and sensitivity).

## Project status: Testing. (Nothing guaranteed)

## How to use:

ScrollLock = switching from actual device to next in loop
ScrollLock + F1 = First HID device
ScrollLock + F2 = Second HID device
ScrollLock + F3 = Third HID device
ScrollLock + F4 = Fourth HID device

Serial speed between Arduino DUE and Arduino Micro is 115200 Baud rate and 4bytes per reading.

## Possible issues:

- Arduino DUE USB host not start, try reset Arduino DUE
- Devices not connecting through hub, try connect external power 5V / 2A to Arduino DUE or USB HUB.
- if the mouse cursor position jumps, wait 3 minutes to stabilize or try to reboot the computer (it is partially software fixed, but it is a hardware problem).
- one cable keyboard of two keyboards is lagging, use only one keyboard or two cable keyboards (some wireless keyboards repeatedly sending empty data for keep connection, that could make problem with other keyboards)
- my mouse has low sensitivity, try disable debugging (bool debug = false;)

## Schematic

## Photos

