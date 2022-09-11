# Spin Coater - Hardware Version 3 <br> (Arduino and Rotary Encoder)
This spin coater version makes use of a MOSFET and an Arduino nano to control a DC motor in a 2-stage program. A rotary encoder with a built in momentary contact switch is naviage and set program parameters which are displayed on a LCD screen, specifically speed 1, speed 2, time 1, time 2 and ramp time.

## Spin coater code Version 2.x

## Planned imporvements
(To Do) Function: Check calibration and set PWM ramp if absent
(To Do) Convert rotary encoder from delay based mechanical bounce/noise handling to time a time difference based wait
        - Currently setting parameters is easy and efficient while setting PWM limits requires slower operation

## Change log v2.3.1 (Sept2022)
- Set to Defaults function added in options menu
- PWM limit parameters:
- - variable "sMax" the max PWM values was set at a low byte value:
- - This was corrected and a menu option was added to adjust PWM limit values in the Menu
- - Added sMin, sMax to set defaults function
- - Calibration/settings warning for bad RPMlookup values from PWM limits, sMin and sMax
- Read rotation of the rotary encoder moved to a discrete function
- - This could then be used for both parameter setting  and setting PWM limits in the options menu
- Bug fixes on value sanitisation (run parameters etc.)
- - Bugfix: speed could not be set to 0 (code v1.x doesn't encounter this issue, the patch should still be applied)
check must be: sMin > 1
- Arduino IDE auto-format to clean up in-line spacing
- Removed unused variables: int a[5], int b[5], byte da[5] byte dacc

## Change log v2.3.0 (Aug2022)
- Licence added
- Code clean-up (incl - Bubble sort removed)
- Display refresh interval reduced fro 300 to 200 mS in line with v1.4 (hV2)

## Change log v2.2.2
- Tachometer, long intervals between interrupts are passed over and no longer included in the average RPM 
- General code clean up
- Boost feature added to allow rotation to start at lower speeds