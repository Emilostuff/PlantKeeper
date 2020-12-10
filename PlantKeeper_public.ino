/*
   PLANT KEEPER V1.0
   by Emilostuff – Emil Skydsgaard, 2020

   Software is published for illustrative/learning purposes only.
   Feel free to reuse and modify for your own project.

   DISCLAIMER: ANY USE OF THIS CODE IS AT YOUR OWN RISK!

   Libraries needed:
   - BLYNK: http://help.blynk.cc/en/articles/512105-how-to-install-blynk-library-for-arduino-ide
   - Time: https://github.com/PaulStoffregen/Time
   - ADS1115_WE: Found in the builtin library manbager

   And you need to have installed support for esp8266 boards

   Enjoy!

*/

// Libraries
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include<ADS1115_WE.h>
#include<Wire.h>
#include <TimeLib.h>
#include <WidgetRTC.h>

// Networking
char auth[] = "Blynk token";   
char ssid[] = "Network name";
char pass[] = "Password";

// Blynk
BlynkTimer timer;
BlynkTimer timer1;
WidgetRTC rtc;

// System state
int plantSelect = 0;                            // plant 1 selected on default (index 0)
bool systemOn = 0;
bool systFlag = false;
long ontime;
long lastWater[4] = {0, 0, 0, 0};               // 0 means not set
bool pumpOn[4] = {false, false, false, false};

// Settings (can be adjusted from app)
int modes[4] = {0, 0, 0, 0};
int amount[4] = {0, 0, 0, 0};
int interval[4] = {0, 0, 0, 0};
int thresh[4] = {0, 0, 0, 0};
int minInterval[4] = {0, 0, 0, 0};

// sensor/ADC
ADS1115_WE adc(0x48);
float tf = 0.1;                                 // trust factor for smoothing filter
float sensor[4] = {100, 100, 100, 100};         // Set highest start value to avoid unwanted triggers
float sensorDry[4] = {2760, 2680, 2780, 2760};  // Reading from when fully emerged in water
float sensorWet[4] = {1460, 1210, 1510, 1500};  // Reading from when in 'dry' air


// BLYNK ///////////////////////////////////////

BLYNK_CONNECTED()
{
  // Synchronize unix-time on connection
  rtc.begin();
}


// IN-APP EVENT CALLS ///////////////////////
// for when the user presses any button in the app

// System on-off button event
BLYNK_WRITE(V7)
{
  // change system state
  systemOn = param.asInt();

  if (systemOn) {
    if (systFlag) {
      // system has just been turned on!
      systFlag = false;

      // set lastWater to now:
      ontime = now();
      Blynk.virtualWrite(V34, ontime, ontime, ontime, ontime);
      Blynk.syncVirtual(V34);
    }
    // system was turned on when connected -> do nothing
  } else {
    // system is off
    systFlag = true;
  }
}

// Manual water button event
BLYNK_WRITE(V8)
{
  // if button was pressed and plant is eligible for water
  if (param.asInt() == 1 and now() - lastWater[plantSelect] > 5 and systemOn) {
    // execute water routine
    water(plantSelect);
  } else {
    // reset water button to unpressed state
    Blynk.virtualWrite(V8, 0);
  }
}

// Reload button event
BLYNK_WRITE(V2) 
{
  // Reload requested -> update display values in app
  if (param.asInt() == 1); {
    Blynk.virtualWrite(V1, modes[plantSelect]);
    Blynk.virtualWrite(V3, amount[plantSelect]);
    Blynk.virtualWrite(V4, interval[plantSelect]);
    Blynk.virtualWrite(V5, thresh[plantSelect]);
    Blynk.virtualWrite(V9, minInterval[plantSelect]);
  }
}

// Plant Select event
BLYNK_WRITE(V0)
{
  // Plant selected for edit -> store updated value
  plantSelect = param.asInt() - 1;

  // update displays
  Blynk.virtualWrite(V1, modes[plantSelect]);
  Blynk.virtualWrite(V3, amount[plantSelect]);
  Blynk.virtualWrite(V4, interval[plantSelect]);
  Blynk.virtualWrite(V5, thresh[plantSelect]);
  Blynk.virtualWrite(V9, minInterval[plantSelect]);
}

// Mode select event
BLYNK_WRITE(V1)
{
  // Store updated value depending on selected plant
  modes[plantSelect] = param.asInt();

  // save updated value on server
  Blynk.virtualWrite(V30, modes[0], modes[1], modes[2], modes[3]);
}

// Amount change event
BLYNK_WRITE(V3)
{
  // Store updated value depending on selected plant
  amount[plantSelect] = param.asInt();

  // save to server
  Blynk.virtualWrite(V31, amount[0], amount[1], amount[2], amount[3]);
}

// Interval change event
BLYNK_WRITE(V4)
{
  // Store updated value depending on selected plant
  interval[plantSelect] = param.asInt();

  // save to server
  Blynk.virtualWrite(V32, interval[0], interval[1], interval[2], interval[3]);
}

// Threshold change event
BLYNK_WRITE(V5)
{
  // Store updated value depending on selected plant
  thresh[plantSelect] = param.asInt();

  // save to server
  Blynk.virtualWrite(V33, thresh[0], thresh[1], thresh[2], thresh[3]);
}

// Minimum interval change event
BLYNK_WRITE(V9)
{
  // Store updated value depending on selected plant
  minInterval[plantSelect] = param.asInt();

  // save to server
  Blynk.virtualWrite(V35, minInterval[0], minInterval[1], minInterval[2], minInterval[3]);
}



// GET CALLS //////////////////////////////
// used when 'sync' is called at startup 

BLYNK_WRITE(V30)
{
  // Get values from server:
  for (int i = 0; i < 4; i++) {
    modes[i] = param[i].asInt();
  }
}

BLYNK_WRITE(V31)
{
  // Get values from server:
  for (int i = 0; i < 4; i++) {
    amount[i] = param[i].asInt();
  }
}

BLYNK_WRITE(V32)
{
  // Get values from server:
  for (int i = 0; i < 4; i++) {
    interval[i] = param[i].asInt();
  }
}

BLYNK_WRITE(V33)
{
  // Get values from server:
  for (int i = 0; i < 4; i++) {
    thresh[i] = param[i].asInt();
  }
}

BLYNK_WRITE(V35)
{
  // Get values from server:
  for (int i = 0; i < 4; i++) {
    minInterval[i] = param[i].asInt();
  }
}

// Last Water - retreive values from server
BLYNK_WRITE(V34)
{
  // Get values from server:
  for (int i = 0; i < 4; i++) {
    lastWater[i] = param[i].asInt();
  }
}



// STATUS DISPLAY FUNCTIONS /////////////////////
// when app request current plant status 

BLYNK_READ(V11)
{
  Blynk.virtualWrite(V11, getStatus(0));
}
BLYNK_READ(V12)
{
  Blynk.virtualWrite(V12, getStatus(1));
}
BLYNK_READ(V13)
{
  Blynk.virtualWrite(V13, getStatus(2));
}
BLYNK_READ(V14)
{
  Blynk.virtualWrite(V14, getStatus(3));
}
