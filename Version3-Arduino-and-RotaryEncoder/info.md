# Spin Coater - Hardware Version 3 <br> (Arduino and Rotary Encoder)
This spin coater version makes use of a MOSFET and an Arduino nano to control a DC motor in a 2-stage program. A rotary encoder with a built in momentary contact switch is naviage and set program parameters which are displayed on a LCD screen, specifically speed 1, speed 2, time 1, time 2 and ramp time.

## Spin coater code Version 2.x

## Change log v2.3
- Licence added
- Code clean-up (incl - Bubble sort removed)
- Display refresh interval reduced fro 300 to 200 mS in line with v1.4 (hV2)

## Change log v2.2.2
- Tachometer, long intervals between interrupts are passed over and no longer included in the average RPM 
- General code clean up
- Boost feature added to allow rotation to start at lower speeds