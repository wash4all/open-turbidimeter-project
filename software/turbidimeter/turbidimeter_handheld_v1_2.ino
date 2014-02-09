//uncomment this section if including the GSM modem
//#define NO_PORTD_PINCHANGES
//#define NO_PORTC_PINCHANGES
//#include <GSM.h>

#include <PinChangeInt.h>
#include <PinChangeIntConfig.h>
#include <EEPROM.h>
#include "EEPROMAnything.h"

/********general defines********/
#define PINNUMBER "1111"
#define VERSION_BYLINE "Open Turb Prj\nBIOS v1.8\n2014-01-25\n"
#define IR_LED   13    //light source
#define TSL_S1   12    //S1 and S0 are pins on the TSL230R chip -- see TSL230R user's guide for more information
#define TSL_S0   11
#define TSL_FREQ  4    //the frequency signal from the sensor
#define BPIN     A5    //external button
#define VPIN     A4    //reading from the voltage divider, to gauge voltage by comparison to ATMega328P's internal 1.1v 
#define DIV_R1  1000   //R1 and R2 form the voltage divider, which yields V_out = V_in * R2 / (R1 + R2)
#define DIV_R2 10000    
#define DIVISOR  11    //DIVISOR = (R1+R2)/R1
#define SAMPLING_WINDOW 1000    //milleseconds between frequency calculations
#define HIGH_SENSITIVITY  100   //sensitivity settings, set via S0 and S1 pins
#define MED_SENSITIVITY   10
#define LOW_SENSITIVITY   1
#define READ_REPS 6    //number of replicate readingd the turbidimeter will take per button press (average is used for reporting)

//uncomment these variables if including the GSM modem
//GSM gsmAccess;
//GSM_SMS sms;
boolean notConnected = true;
char* remoteNum = "14105555555";
char* selfNum = "15125555555";
String sNum = "151255555555";

/**********globals**************/
boolean debug = false;  //to update EEPROM, change this to true
int   scale_divider = 2, sensitivity = HIGH_SENSITIVITY;     //scale_divider must match hardwired TSL_S2 and TSL_S3 settings, sensitivity sets TSL_S0 and TSL_S1 settings -- see TSL230R user's guide for more information
int bpress = 1023;                                           //variable for digital mapping of analog button press event
float div_fact = 1;                                          //division factor, to normalize sensor output by voltage level
//long freq_jump_hi = 50000, freq_jump_lo = 4000;            //(These settings could be used to dynamically set sensor sensitivity, e.g. for detecting very bright light levels)
unsigned long timer, frequency;                              
volatile unsigned long pulse_count = 0;                       //interrupt variable; stores count data coming from sensor
boolean bpressed = false, sufficient_battery = true, using_modem = false;
const int num_displays = 4;                                  
const int shift_latch = 5;  //RCLK                           //These three pins connect the 74HC595 shift register
const int shift_clock = 8;  //SRCLCK
const int shift_data = 6;   //SER
const int dispPorts[num_displays] = {A3,A2,A1,A0};
String language = "english"; //"espanol";
//Switch out commented and uncommented to switch from common cathode to common anode seven-segment display
//const byte SevenSegNumbers[13] = {B11000000,  B11111001,  B10100100,  B10110000,  B10011001,  B10010010,  B10000010,  B11111000,  B10000000,  B10010000, B01111111, B10111111, B11111111};  //characters: 0-9, '.', '-', blank
const byte SevenSegNumbers[13] = {  B00111111,  B00000110,  B01011011,  B01001111,  B01100110,  B01101101,  B01111101,  B00000111,  B01111111,  B01101111, B10000000, B01000000, B00000000};  
//const byte SevenSegLetters[8] =  {  B10101111,  B10000110,  B10100001,  B10010001,  B11000111,  B10010010,  B10000111,  B11000000};                                                         //characters: rEdYLStO
const byte SevenSegLetters[8] =  {  B01010000,  B01111001,  B01011110,  B01101110,  B00111000,  B01101101,  B01111000,  B00111111};
/****************************************/
struct config_t{ // for storing data & retrieving data persistently
  int foo;           //example
  long machine_id;   //example
  unsigned long last_calibration_timestamp; // in seconds since 1/1/1970 12:00a
  float y0;     //for the following 4 calibration ranges, yn is the range lower bound
  float m0;     //m is the slope
  float b0;     //and b is the y-intercept
  float y1;
  float m1;
  float b1;
  float y2;
  float m2;
  float b2;
  float y3;
  float m3;
  float b3;
  float y4;
  float m4;
  float b4;
} 
config;
/******************setup and loop*********************/
void setup() { 
  pinMode(TSL_FREQ, INPUT);        //set up inputs and outputs
  pinMode(TSL_S0, OUTPUT);  
  pinMode(TSL_S1, OUTPUT);
  pinMode(IR_LED, OUTPUT);
  pinMode(BPIN, INPUT);
  pinMode(VPIN, INPUT);
  pinMode(shift_latch, OUTPUT); 
  pinMode(shift_clock, OUTPUT);
  pinMode(shift_data,  OUTPUT);
  for(int i = 0; i < num_displays; i++){pinMode(dispPorts[i], OUTPUT);}    
  digitalWrite(IR_LED, LOW);                                               //turn off the light source for now
  //Serial.begin(9600);
  delay(200);
  turnOffDisplay();                                                        //turn off display for now
  setSensitivity(sensitivity);                                             //set sensor sensitivity
  timer = millis();
  displayForInterval(-1, "dashes", 1000);                                  
  if(divisionFactor_TSL230R() < 0){
    sufficient_battery = false;
    displayForInterval(-1, "error", 2000);
  }else{
    
    displayForInterval(-1, "ready", 2000);
    turnOffDisplay();
  }
  
  if(debug){
    config.foo = 255;                               //EEPROMAnything seems to need the struct to start with a integer in [0,255]
    config.machine_id = 11111111;                   //example
    config.last_calibration_timestamp = 1390936721; // in seconds since 1/1/1970 12:00a
    config.y0 = 0;                                  //for the following 4 calibration ranges, yn is the range lower bound
    config.m0 = 0.02876;                              //m is the slope
    config.b0 = -2.224;                              //and b is the y-intercept
    config.y1 = 87.63;
    config.m1 = 0.039;
    config.b1 = -3.124;
    config.y2 = 276.94;
    config.m2 = 0.04688;
    config.b2 = -5.298;
    config.y3 = 2472.3;
    config.m3 = 0.05223;
    config.b3 = -18.525;
    config.y4 = 6049;
    config.m4 = 0.0721;
    config.b4 = -138.9;
    EEPROM_writeAnything(0, config);
  }else{EEPROM_readAnything(0, config);}
}

void loop() {
  if(sufficient_battery){
    bpress = analogRead(BPIN);   //check for button press event
    if (bpress == 0){            //if the external button was pressed
      bpressed = true;
      divisionFactor_TSL230R();                //discard first reading
      div_fact = divisionFactor_TSL230R();     //take another reading of voltage divider
      if(div_fact < 0){sufficient_battery = false;}     
      else{                                                  //take readings if there is enough battery power for the  sensor
        float reading = takeReadings(READ_REPS);             //read sensor value for given number of times
        displayForInterval(reading, "data",4000);            //display the resulting value for 4000 milliseconds.
        displayForInterval(-1, "clear", 100);                //clear out register pins, for a clean display next time device is powered off/on.
        if(using_modem){                                     //build a text message and transmit, if using_modem flag is set to true
          int msg_len = 140;
          char txtMsg[msg_len];
          String bn, message_text;
          bn = baseNmap(reading);
          message_text = "#cod debug #con xxxxxxxxxxx #mtac " + bn;
          /*******************Uncomment this section if including the gsm.******************
          //This command as currently coded will send a coded text message of every reading
          openConnection();
          delay(30000);
          sendMessage(selfNum, message_text);
          closeConnection();
          *********************************************************************************/
        }
      }
    }
    if(bpressed){
      bpressed = false;
      turnOffDisplay();
    }
  }else{
    divisionFactor_TSL230R();
    displayForInterval(-1, "error", 500);
    turnOffDisplay();
    delay(500);
  }
}

/***************voltage meter functions***************/
float divisionFactor_TSL230R(){
  float m = .0052;   //slope of sensor's linear response curve
  float vmin = 3.0;  //min operating v of sensor
  float vmax = 5.5;  //max operating v of sensor
  float v100 = 4.9;  //voltage | normalized response of TSL230r = 1.0
  analogReference(INTERNAL); 
  delay(200); 
  float v = getVoltageLevel();
  analogReference(DEFAULT);
  delay(200); 
  if(v < vmin || v > vmax){return -1;}
  else{return 1 - (4.9 - v) * m;} 
}

float getVoltageLevel(){
  int sensorValue = analogRead(VPIN); //ditch the first reading
  delay(100);
  sensorValue = analogRead(VPIN);
  float voltage = float(sensorValue)/ 1023.0 * 1.1 * 11;  //normalize by max mapping value, internal reference voltage, and voltage divider, respectively.
  return voltage;
}
/******************SevSeg functions*********************/
void turnOffDisplay(){
  for(int i = 0; i < num_displays; i++){digitalWrite(dispPorts[i], HIGH);}  //turn off digit pins 
}

void DisplayADigit(int dispnum, byte digit2disp){  
  digitalWrite(shift_latch, LOW);                            //turn shift register off
  turnOffDisplay();
  shiftOut(shift_data, shift_clock, MSBFIRST, digit2disp);   //perform shifting
  digitalWrite(shift_latch, HIGH);                           //turn register back on
  digitalWrite(dispnum, LOW);
  delay(2);                                                  //for persistance of vision
}

void SevenSegDisplay(float f, String msg){
  if(msg == "data"){
    long powers[6] = {10000, 1000, 100, 10, 1};
    if(f > 9999){f = 9999.0;}  //bounds checks for display
    if(f < 0){f = 0.0;}
    int numeric_scale = 1, pt = -1, start = 0;  //determine where to put decimal point and leading blank digit, if needed
    if(f > 1000){;}
    else if(f > 100){
      numeric_scale = 10;
      pt = 2;
    }else if(f > 10){
      numeric_scale = 100;
      pt = 1;
    }else if(f > 1){
      numeric_scale = 100;
      pt = 1;
      start = 1;
      DisplayADigit(dispPorts[0],SevenSegNumbers[0]);
    }else{
      numeric_scale = 100;
      start = 2;
      DisplayADigit(dispPorts[0],SevenSegNumbers[12]);
      DisplayADigit(dispPorts[1],byte(SevenSegNumbers[0] | SevenSegNumbers[10]));
    } 
    long f2l = long(f * numeric_scale);
    for(int i = start; i < 4; i++){
      if(i == pt){DisplayADigit(dispPorts[i],SevenSegNumbers[(f2l% powers[i]) / powers[i+1]] | SevenSegNumbers[10]);}  //perform modulo and integer division calculations to separate digits; bit mask with decimal point if needed
      else{DisplayADigit(dispPorts[i],SevenSegNumbers[(f2l% powers[i]) / powers[i+1]]);}                               //do the above line this way if decimal point not needed for given digit
    }
  }else if(msg == "dashes"){
    for(int i = 0; i < num_displays; i++){DisplayADigit(dispPorts[i],SevenSegNumbers[11]);}
  }else if(msg == "cycle_dashes"){
    for(int i = 0; i < num_displays; i++){
      DisplayADigit(dispPorts[i],SevenSegNumbers[11]);
      delay(100);
    }
  }else if(msg == "ready"){    //display best available approximation of "ready" message, in Spanish or English, on seven-segment display
    if(language == "english"){
      DisplayADigit(dispPorts[0],SevenSegLetters[0]);  //r
      DisplayADigit(dispPorts[1],SevenSegLetters[1]);  //E
      DisplayADigit(dispPorts[2],SevenSegLetters[2]);  //d
      DisplayADigit(dispPorts[3],SevenSegLetters[3]);  //Y
    }else if(language == "espanol"){
      DisplayADigit(dispPorts[0],SevenSegLetters[4]);  //L
      DisplayADigit(dispPorts[1],SevenSegLetters[5]);  //S
      DisplayADigit(dispPorts[2],SevenSegLetters[6]);  //t
      DisplayADigit(dispPorts[3],SevenSegLetters[7]);  //O
    }
  }else if(msg == "error"){  //display best available bilingual approximation of "error" message on seven-segment display
    DisplayADigit(dispPorts[0],SevenSegLetters[1]);  //E
    DisplayADigit(dispPorts[1],SevenSegLetters[0]);  //r
    DisplayADigit(dispPorts[2],SevenSegLetters[0]);  //r
    DisplayADigit(dispPorts[3],SevenSegNumbers[12]);  //
  }
  else if(msg == "clear"){
    for(int i = 0; i < num_displays; i++){DisplayADigit(dispPorts[i],SevenSegNumbers[12]);}    
  }
}

void displayForInterval(float val, String msg, long ms){
    unsigned long timer = millis();
    while(millis() - timer < ms){SevenSegDisplay(val, msg);}
    turnOffDisplay();
}
/******************interrupt and TSL230R functions*********************/
void add_pulse() {pulse_count++;} //this simple function counts pulses sent from the sensor

void setSensitivity(int sens){    //set sensor sensitivity
  if(sens == LOW_SENSITIVITY){  
    digitalWrite(TSL_S0, LOW);
    digitalWrite(TSL_S1, HIGH);
    sensitivity = LOW_SENSITIVITY;
  }else if(sens == MED_SENSITIVITY){
    digitalWrite(TSL_S0, HIGH);
    digitalWrite(TSL_S1, LOW);
    sensitivity = MED_SENSITIVITY;
  }else if(sens == HIGH_SENSITIVITY){
    digitalWrite(TSL_S0, HIGH);
    digitalWrite(TSL_S1, HIGH);
    sensitivity = HIGH_SENSITIVITY;
  }
  return;
}

float takeReadings(int num_rdgs){
  digitalWrite(IR_LED, HIGH);                               //turn on light source
  int rep_cnt = 0, b = 0;
  long sum = 0, low = 1000000, high = 0, rd = 0;
  displayForInterval(-1, "cycle_dashes", 1000);
  PCintPort::attachInterrupt(TSL_FREQ, add_pulse, RISING);  //turn on frequency-counting function
  delay(200);
  pulse_count = 0;                                          //reset frequency counter
  timer = 0;
  while(rep_cnt < num_rdgs){                                //for given number of readings
    if(millis() - timer >= SAMPLING_WINDOW){                //once 1000 ms have elapsed
      rd = pulse_count * scale_divider;                     //normalize frequency by TSL_S2 & TSL_S3 settings
      if(rd > high){high = rd;}                             //find highest and lowest readings in the group
      if(rd < low){low = rd;}
      sum += rd;                                            //and the sum of readings
      timer = millis();                                     //update timer
      rep_cnt++;
      pulse_count = 0;
    }
  }  
  PCintPort::detachInterrupt(TSL_FREQ);                     //turn off frequency-counting function
  digitalWrite(IR_LED, LOW);                                //turn off light source
  if(num_rdgs > 3){                  //chuck out highest and lowest readings and average the rest, if there are four or more readings
    sum -= (high + low);
    b = 2;
  }
  float raw_value = float(sum) / float(num_rdgs - b) / div_fact, ntu_value = -1;    //get average reading, with highest and lowest discarded
  //for much higher turbidities, code below could easily be expanded and sensitivity dynamically adjusted
  if(sensitivity == HIGH_SENSITIVITY){  
    if(raw_value > config.y4)       {ntu_value = raw_value * config.m4 + config.b4;}   //map averaged raw sensor value to NTU
    else if(raw_value > config.y3)  {ntu_value = raw_value * config.m3 + config.b3;}   //using calibration info stored in persistent memory
    else if(raw_value > config.y2)  {ntu_value = raw_value * config.m2 + config.b2;}
    else if(raw_value > config.y1)  {ntu_value = raw_value * config.m1 + config.b1;}  
    else                            {ntu_value = raw_value * config.m0 + config.b0;}
    return ntu_value;
  }else{return 9999;}
}

//baseNmap encodes turbidity values in a base64 cipher, to save space if transmitting many values at once
String baseNmap(float val){
  int value = (int)(val * 100);
  String enc = "";
  String encoding = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ/+";
  int base = encoding.length();
  encoding += encoding.substring(0,1);
  long v = value / base;
  long m = value % base;
  while (v > 0){
    enc = encoding.substring(v%base, v%base+1) + enc;
    v = v / base;
  }
  enc += encoding.substring(m%base, m%base+1);
  return enc;
}

/*******************uncomment this section if including the GSM modem*****************
//NOTE: connect the modem to pins 2 (TX), 3 (RX), 7 (RESET), 5V, and GND.

String sendMessage(char* remoteNum, char* message){
  char* txtMsg = message;
  sms.beginSMS(remoteNum);  // send the message
  sms.print(txtMsg);
  sms.endSMS();
  return "complete";
}

void openConnection(){
  notConnected = true;
  while (notConnected) {
    digitalWrite(3,HIGH);       // Enable the RX pin
    if(gsmAccess.begin(PINNUMBER)==GSM_READY){notConnected = false;}
    else{delay(1000);}
  }
}

void closeConnection(){
  while(notConnected==false){
    if(gsmAccess.shutdown()==1){
      digitalWrite(3,LOW);      // Disable the RX pin
      notConnected = true;
    }
  }
}
***************************************************************************************/
