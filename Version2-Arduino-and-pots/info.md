# Spin Coater Version 2 <br> (Arduino and pots)
This spin coater version makes use of a MOSFET and an Arduino nano to control a DC motor in a 2-stage program. A series of pots are used to control the program parameters, specifically speed 1, speed 2, time 1, time 2 and ramp time.

## Change log v1.4
- showRPM from "int" to "void", previously returned RPM
- showRPM updated as per v2.2.2 (hV3) to better deal with 0 or near 0 rpm
- updated "book checkRed()" function

## Change log from v1.2 to v1.3
- Move from 2 step to 1 step to start a run
- Show all parameters during setup as in done for v2.1(hV3)
- Arrow to mark parameter being altered with timeout for no parameter being changed
- (bug fix) PWM value 31 difficult to select with dacc of 5: pot2PWM changed the maximum value from from 242 to 201
- Calibration shortened from 30 to 20 s cool downs, from 10 to 8 s stabilisation time, from 1000 to 800 mS between measurements.
- Display interval from 300 to 200 mS
- ShowRPM changed from void to return the RPM as an int
- Bubble sort code removed