/*
Code for Spin coater hV2

Code Version  1.4.x

Copyright 2022 Adam Shnier

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

The above license excludes snippets of code from external sources which have been referenced in comments where used.

There are 4 lines of BSD licenced code (excl. white space) and associated comments for driving the display from an Adafruit example sketch. https://github.com/adafruit/Adafruit-PCD8544-Nokia-5110-LCD-library/blob/master/license.txt (accessed Aug 2022)
There are 9 lines of code for EEPROM int handling (excl. white space) from the "garage blog of Marius aka Scriblab[.se]" where the disclaimer below is found at http://electronics.scriblab.de/sample-page/ (accessed Aug2022)
        "General 
            You do use all the provided information at your own risk. The blog owner is not responsible for any damage that may occur to you while using the information provided. " end of quoted disclaimer
*/

#include <EEPROM.h>
// The calibration is upto (255*2)+1 = 511
// bSPDcorr takes address 512
// bRPM4ramp takes address 513
// bRPM4set takes address 514

// bSPDcorr is not in use. It is a place marker for a future speed correction algorthm
// bRPM4ramp Ramping is linear in terms of True:RPM or False:duty-cycle
// bRPM4set True: parameter pot set speed by RPM, False: speed set by PWM duty cycle (255 = 100%) 
  
// Display
float cali[5]; // used for calibration RPM readings and averaging analog read values pre-run
byte j = 0;
// byte k = 0; // reserve for showRPM // moved to be local
// byte m = 0; // reserve for IRsense // moved to be local
// byte n = 0; // reserve for PWMlookup // moved to be local

/*********************************************************************
BSD licensed code snippet from https://github.com/adafruit/Adafruit-PCD8544-Nokia-5110-LCD-library/blob/master/examples/pcdtest/pcdtest.ino (accessed Aug2022)
*********************************************************************/
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>

// Hardware SPI (faster, but must use certain hardware pins):
// SCK is LCD serial clock (SCLK) - this is pin 13 on Arduino Uno
// MOSI is LCD DIN - this is pin 11 on an Arduino Uno
// pin 5 - Data/Command select (D/C)
// pin 4 - LCD chip select (CS)
// pin 3 - LCD reset (RST)
Adafruit_PCD8544 display = Adafruit_PCD8544(5, 4, 3);
// Note with hardware SPI MISO (pin12) and SS (pin10) pins aren't used but will still be read
// and written to during SPI transfer.  Be careful sharing these pins!
// End of BSD licenced code snippet
// =====================================================================

// RPM variables
float tachOld = micros();
float c[18]; // array from which the displayed speed is an average
float c0 = 100000; // used to check for a non zero value before updating "c"
float cTotal = 600000;
unsigned long tRPMnull; // value given in Setup
int rpm = 0;

int i; // loop counter

  // name of analog read pins
const uint8_t p[5] = {A6, A5, A4, A3, A2};
  // Spin variable array
int a[5];
int b[5];

byte s1, s2;
int RPMs1,RPMs2;
unsigned int t1, t2, ramp;
unsigned long tRampOld = 0;

//differences 
// for each pot the difference between the current and prevoius value is noted
// if the difference is smaller than dacc the change is ignored, this is to stabilise values.
byte da[5] = {0,0,0,0,0};
const byte dacc = 5;

float ramprPWM, ramprRPM;
unsigned long tStart = 0; // used in calibrate and the main loop

bool bStart;
int nStage = 0;
bool bRun = 0;
bool bSPDcorr = 0;
bool bRPM4ramp = 1;
bool bRPM4set = 0;
bool bReport = 0;
bool bSame = 0; // this is a variable that will get set before each time it is used
// end times for each stage/step
unsigned long t1p = 0;
unsigned long t2p = 0;
unsigned long tr1p = 0;
unsigned long tr2p = 0;
unsigned long tr1 = 0;
unsigned long tr2 = 0;
unsigned long tNext = 0;
unsigned long tCurr = 500; // general timing
unsigned long tDispOld = 0;
unsigned long tSPDcorr = 0;
unsigned long tReport = 0;
unsigned long tSerialOld = 0;

//
byte spd = 0;
byte spd2 = 0;
const int pwm = 6; // Pin for motor control
const int btnStart = 7; // Pin for push to start

bool bFunc = 0;

byte pos = 99;
unsigned long tPosCng = 0; // time of last "pos" change

String sSerial;

// Pos refers to what parameter is currently being changed with the parameter pots.

// =================
// Speed calibration
// =================
//const int duty[242] = {13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127, 128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143, 144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159, 160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175, 176, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191, 192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207, 208, 209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223, 224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239, 240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254, 255, 256};
// meas is the speed divided by 30 starting from PWM13 for the byte type array
//byte meas[243] = {13, 16, 18, 21, 24, 26, 29, 32, 35, 38, 41, 44, 47, 50, 53, 56, 59, 62, 65, 68, 71, 73, 76, 79, 82, 84, 87, 90, 92, 95, 97, 100, 103, 105, 107, 110, 112, 114, 117, 118, 121, 123, 125, 127, 129, 131, 134, 136, 137, 139, 141, 143, 144, 146, 148, 149, 151, 153, 154, 156, 157, 158, 160, 162, 163, 164, 166, 166, 166, 166, 166, 172, 173, 175, 176, 177, 178, 179, 181, 182, 183, 184, 185, 186, 187, 188, 188, 189, 190, 191, 191, 192, 193, 194, 195, 196, 197, 198, 198, 199, 200, 201, 201, 203, 203, 203, 204, 205, 205, 206, 207, 207, 208, 208, 209, 209, 210, 211, 211, 212, 212, 213, 214, 214, 214, 215, 215, 216, 216, 216, 217, 217, 217, 218, 218, 218, 219, 219, 219, 220, 220, 220, 221, 218, 218, 220, 220, 220, 221, 221, 222, 222, 222, 223, 223, 224, 224, 224, 225, 225, 225, 226, 226, 226, 227, 227, 227, 227, 227, 228, 228, 228, 228, 229, 229, 229, 230, 230, 230, 230, 230, 231, 232, 232, 232, 232, 232, 232, 232, 233, 228, 230, 231, 231, 231, 231, 232, 232, 232, 232, 233, 233, 233, 234, 234, 234, 233, 234, 235, 235, 235, 236, 235, 236, 237, 237, 236, 238, 238, 238, 239, 238, 238, 239, 238, 239, 240, 241, 241, 241, 241, 241, 242, 242, 242, 242, 243, 243, 244, 245, 246, 248, 0};
//int meas[243] = {400, 495, 565, 633, 715, 798, 885, 966, 1057, 1147, 1239, 1324, 1408, 1497, 1593, 1689, 1771, 1861, 1954, 2047, 2127, 2207, 2289, 2374, 2463, 2538, 2616, 2700, 2773, 2847, 2919, 3001, 3089, 3166, 3228, 3297, 3373, 3438, 3507, 3564, 3634, 3707, 3769, 3834, 3895, 3951, 4016, 4076, 4119, 4178, 4238, 4292, 4339, 4387, 4442, 4495, 4533, 4586, 4634, 4685, 4712, 4759, 4810, 4857, 4887, 4937, 4979, 4983, 4976, 4977, 4980, 5171, 5203, 5259, 5275, 5315, 5351, 5382, 5427, 5459, 5491, 5523, 5555, 5576, 5606, 5636, 5648, 5677, 5711, 5730, 5751, 5777, 5803, 5829, 5867, 5885, 5910, 5938, 5956, 5978, 6009, 6025, 6053, 6087, 6093, 6113, 6143, 6151, 6170, 6204, 6207, 6223, 6237, 6258, 6281, 6290, 6313, 6331, 6336, 6359, 6373, 6395, 6415, 6420, 6438, 6457, 6462, 6477, 6487, 6496, 6516, 6522, 6534, 6548, 6550, 6565, 6566, 6580, 6587, 6614, 6621, 6621, 6639, 6540, 6564, 6599, 6620, 6621, 6648, 6645, 6658, 6675, 6684, 6702, 6709, 6722, 6740, 6741, 6755, 6764, 6765, 6788, 6785, 6802, 6807, 6809, 6826, 6826, 6832, 6840, 6842, 6856, 6863, 6875, 6884, 6880, 6905, 6903, 6914, 6923, 6923, 6950, 6957, 6962, 6964, 6973, 6970, 6972, 6980, 6989, 6859, 6917, 6928, 6948, 6931, 6951, 6962, 6966, 6984, 6982, 6989, 7000, 7004, 7030, 7035, 7036, 7006, 7039, 7069, 7061, 7072, 7082, 7071, 7104, 7106, 7107, 7101, 7139, 7152, 7159, 7189, 7141, 7159, 7166, 7164, 7186, 7195, 7232, 7235, 7243, 7247, 7253, 7261, 7261, 7278, 7284, 7295, 7305, 7324, 7351, 7375, 7444, 0};

// =================
// =================
void setup() {
  // The calibration is upto (255*2)+1 = 511
  bSPDcorr = bool(EEPROM.read(512)); 
  bRPM4ramp = bool(EEPROM.read(513)); 
  bRPM4set = bool(EEPROM.read(514)); 
  // rpm values
  tRPMnull = 1000000;
  for (i=0; i < 18; i++){
    c[i] = tRPMnull;
  }
    // initialize set parameters
  for (i=0; i < 5; i++){
    a[i] = analogRead(p[i]);
  }
  if (bRPM4set) {
    RPMs1 = pot2RPM(a[0]);
    RPMs2 = pot2RPM(a[2]);
    s1 = PWMlookup(RPMs1);
    s2 = PWMlookup(RPMs2);
  } else { 
    s1 = pot2PWM(a[0]);
    s2 = pot2PWM(a[2]);
    RPMs1 = RPMlookup(s1);
    RPMs2 = RPMlookup(s2);
  }
  t1 = (round(float(a[1])*30000/1000000)*1000); //30s //using 1000 instead of 1023
  t2 = (round(float(a[3])*60000/1000000)*1000); //60s

  ramp = (round(float(a[4])*10000/1000000)*1000); //upto 1s per ~10% output speed change
  tRampOld = 0;
  
  
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(pwm, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);

  // Attaches the tachometer pin to an interrupt
  attachInterrupt(digitalPinToInterrupt(2),IRsense,RISING);  //  function for creating external interrupts at pin2 on Rising (LOW to HIGH)
  
  // Display
  Serial.begin(9600);
  display.begin();
  display.setRotation(2);
  display.setContrast(50);
  display.setTextColor(BLACK);
  display.setCursor(0,0);
  hello();

  /* // this was used for testing the code without using self-calibration
  for (i = 0; i < (256-13); i += 1){
    EEPROMWriteInt( (2*i), meas[i]);
  } 
  */ 
}

void loop() {

  // =======================================
  // Print calibration data via USB (serial)
  // =======================================
  if (millis() < 5000){ // Serial slows down the Arduino
    if (Serial){
      sSerial = Serial.readString();
      if ((sSerial != "") and (sSerial != "\n")){
        Serial.print("User@SpinCoater: ");
        Serial.println(sSerial);
      }
      if (sSerial == "calibration\n"){
        for (i = 0; i < (256-13); i += 1){
          Serial.print(byte(i+13));
          Serial.print(" ");
          Serial.println(EEPROMReadInt(2*i));
        }
      } 
    }
  }

  // ============================
  // Update parameters in memory 
  // ============================
  for (i=0; i < 5; i++){
    tStart = millis();  // 
    for (j=0; j < 5; j += 1){
      cali[j] = analogRead(p[i]);
      while ((millis()-tStart) < ((j+1)*5)); // values are only displayed every 300+ millis (5*5 = 25)
    } // Close (j): take readings
    for (j=1; j < 5; j++){
      cali[0] += cali[j];
    } // Close: Sum
    b[i] = round(cali[0]/5); // value/5
    da[i] = abs(a[i]-b[i]);
  }
  
  if (da[0] > dacc){ //dacc is the sensitivity to change, once set at 15
    a[0] = b[0]; //analogRead(p[0]);
    if (bRPM4set){
      bSame = (pot2RPM(a[0]) != RPMs1);
    } else bSame = (pot2PWM(a[0]) != s1);
    if (bSame){
      if (bRPM4set) {
        RPMs1 = pot2RPM(a[0]);
        s1 = PWMlookup(RPMs1);
        pos = 2; // parameter for display
      } else { 
        s1 = pot2PWM(a[0]);
        RPMs1 = RPMlookup(s1);
        pos = 1; // parameter for display
      }
      tPosCng = millis();
    }
  }
  
  if (da[1] > dacc){
    a[1] = b[1]; //analogRead(p[1]);
    if (round(float(a[1])*30000/1000000) != round(t1/1000)){
      t1 = (round(float(a[1])*30000/1000000)*1000); //30s
      pos = 0; // parameter for display
      tPosCng = millis();
    }
  }
  if (da[2] > dacc){ //dacc is the sensitivity to change, once set at 15
    a[2] = b[2]; 
    if (bRPM4set){
      bSame = (pot2RPM(a[2]) != RPMs2);
    } else bSame = (pot2PWM(a[2]) != s2);
    if (bSame){
      if (bRPM4set) {
        RPMs2 = pot2RPM(a[2]);
        s2 = PWMlookup(RPMs2);
        pos = 5; // parameter for display
      } else { 
        s2 = pot2PWM(a[2]);
        RPMs2 = RPMlookup(s2);
        pos = 4; // parameter for display
      }
      tPosCng = millis();
    }
  }
  if (da[3] > dacc){
    a[3] = b[3]; // analogRead(p[3]);
    if ((round(float(a[3])*60000/1000000)*1000) != t2){
      t2 = (round(float(a[3])*60000/1000000)*1000); //60s
      pos = 3; // parameter for display
      tPosCng = millis();
    }
  }
  if (da[4] > dacc){
    a[4] = b[4]; // analogRead(p[4]);
    if ((round(float(a[4])*10000/1000000)*1000) != ramp){
    ramp = (round(float(a[4])*10000/1000000)*1000); //10s
    pos = 6; // parameter for display
    tPosCng = millis();
    }
  }

  // ==================================
  // Update parameters shown on display 
  // ==================================
  if ((millis() - tDispOld) > 200){
    tDispOld = millis();
    display.clearDisplay();
    display.setTextSize(1);
    display.println("Time   RPM ");
    if (pos == 0){
      display.println(">"+String(t1/1000)+"  "+String(s1)+"  "+String(RPMs1));
    } else if (pos == 1){
      display.println(" "+String(t1/1000)+" >"+String(s1)+"  "+String(RPMs1));
    } else if (pos == 2){
      display.println(" "+String(t1/1000)+"  "+String(s1)+" >"+String(RPMs1));
    } else {
      display.println(" "+String(t1/1000)+"  "+String(s1)+"  "+String(RPMs1));
    }
    if (pos == 3){
      display.println(">"+String(t2/1000)+"  "+String(s2)+"  "+String(RPMs2));
    } else if (pos ==4){
      display.println(" "+String(t2/1000)+" >"+String(s2)+"  "+String(RPMs2));
    } else if (pos ==5){
      display.println(" "+String(t2/1000)+"  "+String(s2)+" >"+String(RPMs2));
    } else {
      display.println(" "+String(t2/1000)+"  "+String(s2)+"  "+String(RPMs2));  
    }
    if (pos == 6){
      display.println("Ramp(s):>" + String(ramp/1000));
    } else {
      display.println("Ramp(s): " + String(ramp/1000));
    }
    display.display();
    
    // if no setting has been altered for XX miliseconds
    if ((millis() - tPosCng) > 1500){
      pos = 99;
    }
  } // Close: Display


  // =====================
  // Start of spin coating
  // =====================
  if (digitalRead(btnStart)){
    if (checkRed()){ // if long press
      menu();
    } else { // if short press
      // =========================
      // Prepare variables for run
      // =========================
      bRun = 1;
      nStage = 0;
      ramprRPM = float(RPMs2)/float(ramp);
      ramprPWM = float(s2)/float(ramp);
      if (s2 > s1){
        if (bRPM4ramp){
          tr1 = float(ramp)*float(RPMs1)/float(RPMs2);
          tr2 = float(ramp)*float(RPMs2-RPMs1)/float(RPMs2); 
        } else {
          tr1 = float(ramp)*float(s1)/float(s2);
          tr2 = float(ramp)*float(s2-s1)/float(s2);
        }
      } else {
        tr1 = ramp;
        tr2 = 0;
      }
  
      tStart = millis(); // start of speed program
      tr1p = tStart + tr1;
      t1p = tr1p + t1;
      tr2p = t1p + tr2;
      t2p = tr2p + t2;
      tNext = tr1p;
    }
    
    // ==========
    // Start Run
    // ==========
    while (bRun){
      if (tStart > 2000){
        // Press again to stop
        if (digitalRead(btnStart)){
          checkRed();
          done();
          break;
      }      
      // determine stage and set speed for fixed speed stages
      tCurr = millis();
      if ((tCurr > tr1p) && (nStage == 0)){
        spd = s1;
        spd2 = s1;
        nStage = 1;
      } else if ((tCurr > t1p) && (nStage == 1)){
        nStage = 2;
      } else if ((tCurr > tr2p) && (nStage == 2)){
        spd = s2;
        spd2 = s2;
        nStage = 3;
      } else if ((tCurr > t2p) && (nStage == 3)){
        done();
        break;
      }
  
      // set speed for ramping stages
      if (bRPM4ramp){
        if (nStage == 0){
          spd = PWMlookup((float(tCurr-tStart))*ramprRPM);
        } else if (nStage == 2){
          spd = PWMlookup(RPMs1 + (float(tCurr-t1p))*ramprRPM);
        }
      } else {
        if (nStage == 0){
          spd = ((float(tCurr-tStart))*ramprPWM);
        } else if (nStage == 3){
          spd = PWMlookup(RPMs1 + (float(tCurr-t1p))*ramprPWM);
        }      
      }
      
      
      analogWrite(pwm,spd);
        
        //I tried using millis() and time intervals
        //that lead to the motor jerking during operation even at low frequency
        //if the interval based PWM code was run without the rest of the program it ran smoothly
        
        
  
      // End: PWM code
     
        // RPM display
      showRPM(); // this uses tCurr

      if (bReport){
        if ((millis() - tReport) > 100){
          Serial.println(String(spd)+" "+String(rpm)+" "+String((millis()-tStart)));
        }
      }
    } // Close: While(bRun) 
    } 
  }
} // Close: loop()

// ============================
// Tachometer
// ============================
void IRsense(){
  c0 = (micros() - tachOld)/1000;
  if (c0 > 2){ // to prevents double readings  // the motor I used is rated 6000 rpm
  for(byte m = 17; m > 0; m = m - 1){
    c[m] = c[m-1];
    }
  c[0] = c0;
  } // Close: for loop
  tachOld = micros();
} // Close: IRsense

void showRPM(){
  float cTotal = 0;
  int cCount = 0;
  tCurr = millis();
  for(byte k=0; k < 18; k++){
    if (c[k] < 500){ // milliseconds
      cCount += 1;
      cTotal += c[k];
    }
  }
  if (cCount > 0){
    rpm = 20000/(cTotal/cCount); // 60000 sec.min^{-1}/ 3 reading per rotation
  } else {
    rpm = 0;
  }
  if ((micros() - tachOld) > (500000)){ // microseconds
    rpm = 0;
    // clear c[i] intervals after 3 seconds
    if ((micros() - tachOld) > (3000000)){ // microseconds
        for (i=0; i < 18; i++){
          c[i] = tRPMnull;
        }
    }
  }
  if (tCurr-tDispOld > 200){ // time in milliseconds

    display.clearDisplay();
    display.setTextSize(2);
    display.println(round(float(rpm)/10)*10); // round to 10's for display 
    display.setTextSize(1);
    display.println(spd);
    display.println(nStage);
    display.println(round((millis()-tStart)/1000));
    display.display();
    tDispOld = tCurr;

  } // Close: RPM display
}
// ============================
// ============================

// ============================
// Calibration and sort
// ============================

void Calibrate(){
      display.clearDisplay();
      display.setTextSize(1);
      display.println("(Start): calibrate");
      display.println(" ");
      display.println("(Reset): exit");
      display.display();
      bStart = 0;
      tStart = millis();
      while (!(bStart) && ((millis() - tStart) < 5000)){ // wait for red button
        bStart = (digitalRead(btnStart));
      }
      if (bStart){
        for (i = 0; i < (256-13); i += 1){
          tStart = millis(); // start of calibration (also used for normal run)
          spd = byte(i+13);
          analogWrite(pwm,spd);
          while ((millis() - tStart) < 8000){ // get to speed
            showRPM();
          } // Close: 5 sec wait
          
          tStart = millis();
          for (j=0; j < 5; j += 1){
            // RPM has either been calculated in the wait loop (1st value) or in the while loop below
            cali[j] = float(rpm);
            while ((millis()-tStart) < ((j+1)*800)){
              showRPM();
            } // Close: While
          } // Close (j): take readings
          
          for (j=1; j < 5; j++){
            cali[0] += cali[j];
          } // Close: Sum
          
          cali[0] = round(cali[0]/5); // {total of rpms}
          nStage = int(cali[0]);
          EEPROMWriteInt( (2*i), nStage);
          
          analogWrite(pwm, 0);
          tStart = millis();
          while ((millis() - tStart) < 20000){ // cooldown
            showRPM();
          }
          
        } // Close: Core calibrate        
      } // Close: if bStart 
      done();    
} // Close: Calibrate

// Menu
void menu(){
  unsigned long tTimeout = millis();
  byte place = 0;
  display.clearDisplay();
  display.setTextSize(2);
  display.println("Menu");
  display.setTextSize(1);
  display.println("(Short): Next");
  display.println("(Long):");
  display.display();
  while ((millis() - tTimeout) < 15000){
    if (digitalRead(btnStart)){
      tTimeout = millis();
      if (checkRed()){ //long press
        if (place == 1){
          hello();
          return;
        } else if (place == 2){
            Calibrate();     
        } else if (place == 5){
            bRPM4ramp = toggle(bRPM4ramp, "RPM for ramp");
            EEPROM.update(513, bRPM4ramp);
        } else if (place == 6){
            bRPM4set = toggle(bRPM4set, "RPM 4 set SPD");
            EEPROM.update(514, bRPM4set);
        }
      } else { // short
        if (place == 0){
          place = 1;
          display.clearDisplay();
          display.setTextSize(2);
          display.println("Exit");
          display.setTextSize(1);
          display.println("");
        } else if (place == 1){
          place = 2;
          display.clearDisplay();
          display.setTextSize(1);
          display.println("Calibrate");
          display.println("");
          display.println("");
        } else if (place == 2){
          place = 5;
          display.clearDisplay();
          display.setTextSize(1);
          display.println("RPM for ramp");
          display.println("-vs duty cycle");
          if (bRPM4ramp){
            display.println("On");
          } else display.println("Off");
        } else if (place == 5){
          place = 6;
          display.clearDisplay();
          display.setTextSize(1);
          display.println("RPM 4 set SPD");
          display.println("-vs duty cycle");
          if (bRPM4set){
            display.println("On");
          } else display.println("Off");
        } else { // return to first entry
          place = 1;
          display.clearDisplay();
          display.setTextSize(2);
          display.println("Exit");
          display.setTextSize(1);
          display.println("");
        }
        display.println("(Short): Next");
        display.println("(Long): Enter");
        display.display();
      } // Close: Short press  
    } // Close: if(digitalRead(btnStart))
  } // Timeout: While
} // void menu()


// Long or short press
bool checkRed(){
  unsigned long tRed;
  tRed = millis();
  while (digitalRead(btnStart));
  tRed = millis() - tRed;
  if (tRed > 1000){
    delay(200);
    return 1; // long press
  } else {
    delay(200);
    return 0;  // short press
  }
}

bool toggle(bool bPara, char sPara[]){
  if (bPara){
    bPara = false;    
  } else bPara = true;
  display.clearDisplay();
  display.setTextSize(1);
  display.println(sPara);
  if (bPara){
    display.println("On");
  } else display.println("Off");;
  display.println("");
  display.println("(Short): Next");
  display.println("(Long): Enter");
  display.display();
  return bPara;
}

void hello(){
  display.clearDisplay();
  display.setTextSize(2);
  display.println("Hello!");
  display.display();
}

void done(){
  analogWrite(pwm, 0);
  bStart = 0;
  nStage = 0;
  bRun = 0;
  spd = 0;
  
  display.clearDisplay();
  display.setTextSize(2);
  display.println("Done");
  display.display();
}


// ============================
// Conversion, read and write
// ============================
// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
// The following is from:
// http://electronics.scriblab.de/write-an-integer-to-the-arduino-eeprom/
// accessed 20191002 
void EEPROMWriteInt(int address, int value)
{
  byte two = (value & 0xFF);
  byte one = ((value >> 8) & 0xFF);
  
  EEPROM.update(address, two);
  EEPROM.update(address + 1, one);
}
 
int EEPROMReadInt(int address)
{
  long two = EEPROM.read(address);
  long one = EEPROM.read(address + 1);
 
  return ((two << 0) & 0xFFFFFF) + ((one << 8) & 0xFFFFFFFF);
}

// End of code snippet from scriblab.de
// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

byte PWMlookup(int rpm2){
  byte out1 = 0;
  if (rpm2 > ((EEPROMReadInt(0))/2)){
    byte upper = 255;
    byte lower = 0;
    byte mid = round((upper + lower)/2);
    while ((upper - lower) > 1){
      mid = round((upper + lower)/2);
      if (rpm2 > EEPROMReadInt(2*(mid))) {
        lower= mid;
      } else upper = mid;
    }
    if ((EEPROMReadInt(2*upper)-rpm2) < (rpm2-EEPROMReadInt(2*lower))){
      out1 = upper;
    } else out1 = lower;
  }
  return (out1+13);
}

int RPMlookup(byte sLook){
  return EEPROMReadInt(2*int(sLook-13));
}

int pot2RPM(float pot){
  return (int) ((round(float(pot)*60/1023)+5)*100);
}

byte pot2PWM(float pot){
  return byte(float(pot)*201/1023)+13; 
  // 242 in V1.2
}

// ============================
// ============================
