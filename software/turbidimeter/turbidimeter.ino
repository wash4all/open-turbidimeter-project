/*----------------OPEN TURBIDIMETER PROJECT---------------*/
/*-------------------------ARDUINO CODE ----------------------*/
/*
Developed by WASH4All, 2013. See wash4all.org for further
information about the project.
*/
/*-------------------------LEGAL--------------------------*/

/* This device and its components are licensed under a
Creative Commons Attribution-NonCommercial-ShareAlike 3.0 
Unported  (CC BY-NC-SA 3.0) licensed. In summary,
that means you are free to share (copy, distribute, and 
transmit) and remix (adapt)the work, but you must adhere to 
the following conditions:

1. Attribution —  You must attribute the work in the manner 
specified by the author or licensor (but not in any way 
that suggests that they endorse you or your use of the work).

2. Noncommercial — You may not sell the device or derivatives
or use this work for other commercial purposes. 

3. Share Alike — If you alter, transform, or build upon this 
work, you may distribute the resulting work only under the 
same or similar license to this one.

You should have received a copy of the license with this code.
If not, see: http://creativecommons.org/licenses/by-nc-sa/3.0
*/

#include <EEPROM.h>
#include <Wire.h>
#include "TSL2561.h"
#include "EEPROMAnything.h"

TSL2561 tsl1(TSL2561_ADDR_LOW); // the 90-deg unit's ADDR line is tied to ground
TSL2561 tsl2(TSL2561_ADDR_FLOAT); // the 180-deg unit's ADDR line is floating
int serialInput = 0; // variable holds most recent keystroke in main loop
const int samples = 2; // number of samples required for calibration
int samples_taken = 0; // how many calibration samples have been taken so far
float turbidity;
float full_90;
float full_180;
float percent_refraction;
const int ledPin = 13;
float NTU[2]; // for use in sampling routine, index starts at 0!
float AU[2]; // for use in sampling routine, index starts at 0!

struct config_t // for storing data & retrieving data persistently
{
  float m;
  float b;
} 
config;

void setup(void) {
  EEPROM_readAnything(0, config);
  Serial.begin(9600);
  pinMode(13, OUTPUT); 
  delay(1000);
  if (tsl1.begin()) { // this is always true for some reason
    Serial.println("Found sensor");
  } 
  else {
    Serial.println("No sensor?");
    while (1){};
  }

  if (Serial.available() >= 0) { // should return -1 if no serial available
    Serial.println("Waiting for input. Press r to read, c to calibrate.");
  }
  // configure TLS2651 luminosity sensor:
  // You can change the gain on the fly, to adapt to brighter/dimmer light situations
  //tsl1.setGain(TSL2561_GAIN_0X);         // set no gain (for bright situtations)
  tsl1.setGain(TSL2561_GAIN_16X);      // set 16x gain (for dim situations)
  tsl2.setGain(TSL2561_GAIN_16X);      // set 16x gain (for dim situations)
  // Changing the integration time gives you a longer time over which to sense light
  // longer timelines are slower, but are good in very low light situtations!
  //tsl1.setTiming(TSL2561_INTEGRATIONTIME_13MS);  // shortest integration time (bright light)
  tsl1.setTiming(TSL2561_INTEGRATIONTIME_402MS);  // medium integration time (medium light)
  tsl2.setTiming(TSL2561_INTEGRATIONTIME_101MS);  // medium integration time (medium light)
  //tsl1.setTiming(TSL2561_INTEGRATIONTIME_402MS);  // longest integration time (dim light)
}

void loop() {
  if (Serial.available() > 0) { // if there's a serial connection, get commands from it
    serialInput = Serial.read(); // get one-byte command option
    Serial.println((char)serialInput); // tell the user what they pressed

    switch (serialInput) { // do an action based on keypress
    case 'r': // read   
      Serial.println("Taking readings...");
      take_readings(20, 100);
      break;
    case 'c': // calibrate    
      calibrate();
      break;
    case 'u': // cancels calibration in progress
      samples_taken = 0;
      break;
    case 's': // saves calibration to persistent memory
      EEPROM_writeAnything(0, config);
      Serial.println("Config saved. Press r to read, c to recalibrate.");
      break;
    default: // if unknown key was pressed
      Serial.println("Invalid input. Press r to read, c to calibrate.");
    }

  } 
}

void take_readings(int reps, int spacing){
  float dark_90 = 0.0; 
  float dark_180 =0.0;
  float far1 = 0.0;  
  float far2 = 0.0;
  float full_90 = 0.0;
  float full_180 = 0.0;
  uint16_t lum = 0.0;
  
  digitalWrite(ledPin, LOW); //measure ambient light level
  lum = tsl1.getLuminosity(TSL2561_FULLSPECTRUM);
    dark_90 = lum;
  lum = tsl2.getLuminosity(TSL2561_FULLSPECTRUM);
    dark_180 = lum;
    
  digitalWrite(ledPin, HIGH); //turn on light & read sample
  for(int i = 0; i < reps; i++){
    lum = tsl1.getLuminosity(TSL2561_FULLSPECTRUM);
    far1 += lum;
    lum = tsl2.getLuminosity(TSL2561_FULLSPECTRUM);
    far2 += lum;
    //delay(spacing);
  }
  digitalWrite(ledPin, LOW); //turn off LED
  full_90 = (far1-dark_90)/(float)reps; //subtract ambient
  full_180 = (far2-dark_180)/(float)reps; //and divide by # samples
  Serial.print("90-deg: ");  
  Serial.println(full_90);
  Serial.print("180-deg: "); 
  Serial.println(full_180);
  turbidity = (full_90 - config.b)/config.m; //calculate from x=(y-b)/m
  Serial.print("Turbidity: "); 
  Serial.print(turbidity); 
  Serial.println(" NTU");
}

void calibrate(){
  float dark_90 = 0.0; 
  float dark_180 =0.0;
  float far1 = 0.0;  
  float far2 = 0.0;
  float full_90 = 0.0;
  float full_180 = 0.0;
  uint16_t lum = 0.0;
  const int readings = 20; // how many readings to take from sensor

  digitalWrite(ledPin, LOW);
  dark_90 = tsl1.getLuminosity(TSL2561_FULLSPECTRUM); 
  dark_180 = tsl2.getLuminosity(TSL2561_FULLSPECTRUM); 

  if (samples_taken < samples){
    samples_taken = samples_taken + 1;
    Serial.println("Loading calibration routine...");
    Serial.println("Enter the NTU for this sample (5s...)");
    delay(5000); // wait for user
    float sample_NTU = Serial.parseFloat(); // works with Arduino serial monitor
    //float sample_NTU = readFloatFromBytes(); // works with Python (see function comments)
    Serial.println(sample_NTU, DEC); // print recorded value
    Serial.println("Reading sensor");
    digitalWrite(ledPin, HIGH);
    
    for(int i = 0; i < readings; i++){ // take #readings amount of samples
      far1 += tsl1.getLuminosity(TSL2561_FULLSPECTRUM);  // read from sensor 1
      far2 += tsl2.getLuminosity(TSL2561_FULLSPECTRUM);  // read from sensor 2
    }
    
    digitalWrite(ledPin, LOW); //turn off LED
    full_90 = (far1 - dark_90)/(float)readings; // subtract ambient light & ...
    full_180 = (far2- dark_180)/(float)readings; // divide by number of sampling intervals

    Serial.print("90-deg: ");  
    Serial.println(full_90);
    Serial.print("180-deg: "); 
    Serial.println(full_180);
    
    NTU[samples_taken-1] = sample_NTU; //NTU for sample
    AU[samples_taken-1] = full_90; //Arbitrary Units for sample (what sensor returns)
    
    int percent = samples_taken  * 100 / samples;
    Serial.print(percent); 
    Serial.print("% complete. ");
    Serial.println("Press c to continue, or u to cancel...");
  }
  else { //calculate slope & intercept of line between pts (y = mx+b)
    float delta_y = AU[1] - AU[0];
    float delta_x = NTU[1] - NTU[0];
    float m = delta_y / delta_x; //slope
    float b = AU[0] - (m * NTU[0]); //intercept
    config.m = m;
    config.b = b;
    samples_taken = 0; //reset counter
    Serial.println("Calibration complete. y=mx+b (NTU = x, AU = y):");
    Serial.print("m: ");
    Serial.println(config.m, DEC);
    Serial.print("b: ");
    Serial.println(config.b, DEC);
    Serial.println("Press s to save calibration for next time."); 
  }
}

float readFloatFromBytes() {
  /* converts 4-byte float sent from python via serial port to Arduino float
   * python commands to send data:
   * > import struct
   * > import serial
   * > ser = serial.Serial('/dev/ttyACM0 or whatever')
   * > ser.write(struct.pack('f', YOURVALUE) 
   * or send the calibrate command and value at the same time:
   * > ser.write('c' + struct.pack('f', YOURVALUE)
   */
  union u_tag {
    byte b[4];
    float fval;
  } 
  u;
  u.b[0] = Serial.read();
  u.b[1] = Serial.read();
  u.b[2] = Serial.read();
  u.b[3] = Serial.read();
  return u.fval; // returns the float
}







