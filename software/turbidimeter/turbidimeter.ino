/*--------------------------OPEN TURBIDIMETER PROJECT------------------------*/
/*------------------------------- ARDUINO CODE ------------------------------*/
/*---Developed by WASH4All, 2014. See wash4all.org for further information.--*/

// Flags
boolean debug = false;  // IMPORTANT to reset EEPROM to deault config > true
boolean using_modem = false; // if not using a GSM modem, change to false
  
// Libraries
// NOTE: Be sure to use the libraries included with this sketch
// (by placing them in your Arduino or Sketchbook folder)
#define NO_PORTD_PINCHANGES
#define NO_PORTC_PINCHANGES
#include <GSM.h>
#include <PinChangeInt.h>
#include <EEPROM.h>
#include <EEPROMAnything.h>
#include <avr/interrupt.h>
#include <avr/power.h>
#include <avr/sleep.h>
#include <avr/io.h>

// Definitions
#define PINNUMBER "1111" // PIN Number for SIM card
#define VERSION_BYLINE "Open Turb Prj\nBIOS v1.10\n2014-03-29\n"
#define IR_LED    4    // light source
#define TSL_S1   12    // S1 and S0 are pins on the TSL230R chip
#define TSL_S0   11
#define TSL_FREQ 13    // frequency signal from the sensor
#define BPIN     A5    // external button
#define BUTTON_EVENT_VALUE 1000

/* 
   Change boolean value, and switch out commented and uncommented the following two 
   pairs of lines below to switch from common cathode to common anode seven-segment display
   NB: For the seven-segment display listed in the Supplementary Materials document, to distinguish
   common cathode and common anode models, look on the side of the display for the numbers '0.56'. 
   If that number is followed by a plus (+) sign, the display is common cathode.
*/

boolean common_cathode_display = true; //if using a common-anode seven segment display, change to false
/*if(common_cathode_display == true)*/ 
#define SEVEN_SEG_NUMBERS {B11000000,  B11111001,  B10100100,  B10110000, B10011001,  B10010010,  B10000010,  B11111000, B10000000,  B10010000,  B01111111,  B10111111, B11111111}
#define SEVEN_SEG_LETTERS {B10101111,  B10000110,  B10100001,  B10010001, B11000111,  B10010010,  B10000111,  B11000000}

/*if(common_cathode_display == false)*/
//#define SEVEN_SEG_NUMBERS {B00111111,  B00000110,  B01011011,  B01001111, B01100110,  B01101101,  B01111101,  B00000111, B01111111,  B01101111,  B10000000,  B01000000, B00000000}
//#define SEVEN_SEG_LETTERS {B01010000,  B01111001,  B01011110,  B01101110, B00111000,  B01101101,  B01111000,  B00111111}


// Gauge voltage by comparison to ATMega328P's internal 1.1v
// R1 and R2 form a voltage divider, with V_out = V_in * R2 / (R1 + R2)
#define VPIN     A4    // read voltage from this pin
#define DIV_R1 10000   // resistance for R1
#define DIV_R2  1000   // resistance for R2
#define SAMPLING_WINDOW 1000    // milleseconds between frequency calculations
#define HIGH_SENSITIVITY  100   // sensitivity settings, set via S0 and S1 pins
#define MED_SENSITIVITY   10    // ...
#define LOW_SENSITIVITY   1     // ...
#define READ_REPS 6    // number of readings the turbidimeter will take per 
// button press (average is used for reporting)

// Set up GSM library
GSM gsmAccess;  //US band:"GSM850_EGSM_DCS_PCS_MODE"
GSM_SMS sms;
boolean notConnected = true;

// Global Vars
int  scale_divider = 2, sensitivity = HIGH_SENSITIVITY;     
// scale_divider must match hardwired TSL_S2 and TSL_S3 settings, 
// sensitivity sets TSL_S0 and TSL_S1 settings
// see TSL230R user's guide for more information
int bpress = 1023;   
//variable for digital mapping of analog button press event
float div_fact = 1;  
//division factor, to normalize sensor output by voltage level
long freq_jump_hi = 50000, freq_jump_lo = 4000;            
// (These settings could be used to dynamically set sensor sensitivity,
// e.g. for detecting very bright light levels)
unsigned long timer, frequency;                              
volatile unsigned long pulse_count = 0;  
//interrupt variable; stores count data coming from sensor
boolean bpressed = false, sufficient_battery = true;
const int num_displays = 4;                                  
const int shift_latch = 5;  // RCLK                           
const int shift_clock = 8;  // SRCLCK
const int shift_data = 6;   // SER
// The above three pins connect the 74HC595 shift register
const int dispPorts[num_displays] = {A3,A2,A1,A0};
String language = "espanol"; //"english";

// Characters: 0-9, '.', '-', blank                   
const byte SevenSegNumbers[13] = SEVEN_SEG_NUMBERS;
  
// Characters: rEdYLStO
const byte SevenSegLetters[8] =  SEVEN_SEG_LETTERS;

// Use this structure for storing data & retrieving data persistently in EEPROM
struct config_t{
  int foo;           //example
  long machine_id;   //example
  unsigned long last_calibration_timestamp; // in seconds since 1/1/1970 12:00a
  // define calibration constants for 5 calibration curves
  // y is the lower bound, m is the slope, b is the the y-intercept (y=mx+b)
  float y0, y1, y2, y3, y4,
  m0, m1, m2, m3, m4,
  b0, b1, b2, b3, b4;
  char* remoteNum;
  char* selfNum;
  String userfn; // First name of the person using the device
  String useremail; // Email address of the person using the device
  char* userpn; // Phone number of the person using the device
  String username; // Username for Open Source Water server
  String password; // Password for Open Source Water server
} config;

/*---------------------------------------------------------------------------*/
/*--------------------------------FUNCTIONS----------------------------------*/
/*---------------------------------------------------------------------------*/

void setup() {

  // Pin I/O settings
  pinMode(TSL_FREQ, INPUT); // light sensor
  pinMode(TSL_S0, OUTPUT);  // light sensor
  pinMode(TSL_S1, OUTPUT);  // light sensor
  pinMode(IR_LED, OUTPUT);  // light source
  pinMode(BPIN, INPUT); // button
  pinMode(VPIN, INPUT); // voltage
  pinMode(shift_latch, OUTPUT); // shift register
  pinMode(shift_clock, OUTPUT); // shift register
  pinMode(shift_data,  OUTPUT); // shift register
  for(int i = 0; i < num_displays; i++){
    pinMode(dispPorts[i], OUTPUT);
    // set display pins to output
  }

  // Startup procedure
  digitalWrite(IR_LED, LOW);  // light source off
  delay(200);
  turnOffDisplay();
  setSensitivity(sensitivity); // set sensor sensitivity
  timer = millis();
  displayForInterval(-1, "dashes", 1000);

  // Prepare sensors  
  if(divisionFactor_TSL230R() < 0){
    sufficient_battery = false;
    displayForInterval(-1, "error", 2000);
  }
  else{
    displayForInterval(-1, "ready", 2000);
    turnOffDisplay();
  }

  if(debug){
    config.foo = 255;                               
    //EEPROMAnything seems to need the struct to start with a integer in [0,255]
    config.machine_id = 11111111; //example
    config.last_calibration_timestamp = 1390936721;
    config.y0 = 0;                                  
    config.m0 = 0.02876;                              
    config.b0 = -2.224;                              
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
    config.remoteNum = "14109278905";  //phone number of Open Source Water SMS gateway
    config.selfNum = "1410*******";  //phone number of your SIM card
    config.userfn = "****";
    config.useremail = "***@****.org";
    config.userpn = "1410*******";
    config.password = "******";
    config.username = "******";
    
    EEPROM_writeAnything(0, config);
    // Write example calibration settings to EEPROM memory
  }
  else{
    EEPROM_readAnything(0, config);
    // Read calibration data from EEPROM memory
  }
}

void loop() {
  sufficient_battery = true;
  if(sufficient_battery){
    bpress = analogRead(BPIN); 
    // check for button press event (0 = pressed)
    if (bpress >= BUTTON_EVENT_VALUE){
      bpressed = true;
      divisionFactor_TSL230R();                
      // read, but discard first reading
      div_fact = divisionFactor_TSL230R();     
      // take another reading of voltage divider
      if(div_fact < 0){
        sufficient_battery = false;
      }     
      else{
        // Given there is enough battery power for the  sensor,
        // - read sensor value for given number of times,
        // - display the resulting value for 4000 milliseconds,
        // - clear out register pins, for a clean display next 
        //   time device is powered off/on.
        // - build a text message and transmit,
    		// - check for and parse incoming messages,
        //   if using_modem flag is set to true.                                                 
        float reading = takeReadings(READ_REPS);   
        displayForInterval(reading, "data",4000);            
        displayForInterval(-1, "clear", 100);

        if(using_modem){                                     
          int msg_len = 140;
          char txtMsg[msg_len];
          String bn, message_text;
          bn = baseNmap(reading);
          message_text = "#un " + String(config.username) + 
                        " #pw " + String(config.password) + 
                        " #reading " + bn;
          // This command as currently coded will send 
          // a coded text message of every reading!!
          openConnection();
          delay(10000);
          sendMessage(config.remoteNum, message_text);
		      String incoming = getMessageText();
  		    parseMessage(incoming);
          closeConnection();
        }
      }
    }
    if(bpressed){
      bpressed = false; // reset
      turnOffDisplay();
    }
  }
  else{
    divisionFactor_TSL230R();
    displayForInterval(-1, "error", 500);
    turnOffDisplay();
    delay(500);
  }
}

/*--------------------------Voltage Meter Functions--------------------------*/

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
  if(v < vmin || v > vmax){
    return 1;
  }
  else{
    return 1 - (4.9 - v) * m;
  } 
}

float getVoltageLevel(){
  float sensorValue = analogRead(VPIN); //drop the first reading
  delay(100);
  sensorValue = float(analogRead(VPIN));
  float divider_value = float(DIV_R2) / float(DIV_R1+DIV_R2);
  float voltage = sensorValue/ 1023.0 * 1.1 / divider_value;  
  // normalize by max mapping value, internal reference voltage, 
  // and voltage divider, respectively.
  return voltage;
}

/*------------------------------SevSeg Functions-----------------------------*/

void turnOffDisplay(){
  for(int i = 0; i < num_displays; i++){
    if(common_cathode_display){digitalWrite(dispPorts[i], LOW);} //turn off digit pins
    else{digitalWrite(dispPorts[i], HIGH);}
  }  
}

void DisplayADigit(int dispnum, byte digit2disp){  
  digitalWrite(shift_latch, LOW);                     //turn shift register off
  turnOffDisplay();
  shiftOut(shift_data, shift_clock, MSBFIRST, digit2disp);     // perform shift
  digitalWrite(shift_latch, HIGH);                     // turn register back on
  if(common_cathode_display){digitalWrite(dispnum, HIGH);}
  else{digitalWrite(dispnum, LOW);}
  delay(2);                                        // for persistance of vision
}

void displayForInterval(float f, String msg, long ms){
  if(msg == "data"){
    long powers[6] = {10000, 1000, 100, 10, 1};
    if(f > 9999){
      f = 9999.0;
    }  //bounds checks for display
    if(f < 0){
      f = 0.0;
    }
    int numeric_scale = 1, pt = -1, start = 0;  
    // determine where to put decimal point and leading blank digit, if needed
    if(f > 1000){
      ;
    }
    else if(f > 100){
      numeric_scale = 10;
      pt = 2;
    }
    else if(f > 10){
      numeric_scale = 100;
      pt = 1;
    }
    else if(f > 1){
      numeric_scale = 100;
      pt = 1;
      start = 1;
      DisplayADigit(dispPorts[0],SevenSegNumbers[0]);
    }
    else{
      numeric_scale = 100;
      start = 2;
      DisplayADigit(dispPorts[0],SevenSegNumbers[12]);
      if(common_cathode_display){DisplayADigit(dispPorts[1], ~byte(SevenSegNumbers[0] ^ SevenSegNumbers[10]));}
      else{DisplayADigit(dispPorts[1],byte(SevenSegNumbers[0] | SevenSegNumbers[10]));}
    } 
    long f2l = long(f * numeric_scale);
    unsigned long timer = millis();
    while(millis() - timer < ms){
      for(int i = start; i < 4; i++){
        if(i == pt){
          if(common_cathode_display){DisplayADigit(dispPorts[i], ~byte(SevenSegNumbers[(f2l% powers[i]) / powers[i+1]] ^ SevenSegNumbers[10]));}
          else{DisplayADigit(dispPorts[i],byte(SevenSegNumbers[(f2l% powers[i]) / powers[i+1]] | SevenSegNumbers[10]));}
          // perform modulo and integer division calculations to separate digits
          // bit mask with decimal point if needed
        }  
        else{
          DisplayADigit(dispPorts[i], SevenSegNumbers[(f2l% powers[i]) / powers[i+1]]);
          //do the above line this way if decimal point not needed for given digit
        }                               
      }
    }
  }
  else if(msg == "dashes"){
    unsigned long timer = millis();
    while(millis() - timer < ms){
      for(int i = 0; i < num_displays; i++){
        DisplayADigit(dispPorts[i],SevenSegNumbers[11]);
      }
    }
  }
  else if(msg == "cycle_dashes"){
    unsigned long timer = millis();
    while(millis() - timer < ms){
      for(int i = 0; i < num_displays; i++){
        DisplayADigit(dispPorts[i],SevenSegNumbers[11]);
        delay(100);
      }
    }
  }
  else if(msg == "ready"){    
    // display best available approximation of "ready" message, 
    // in Spanish or English, on seven-segment display
    unsigned long timer = millis();
    while(millis() - timer < ms){
      if(language == "english"){
        DisplayADigit(dispPorts[0],SevenSegLetters[0]);  //r
        DisplayADigit(dispPorts[1],SevenSegLetters[1]);  //E
        DisplayADigit(dispPorts[2],SevenSegLetters[2]);  //d
        DisplayADigit(dispPorts[3],SevenSegLetters[3]);  //Y
      }
      else if(language == "espanol"){
        DisplayADigit(dispPorts[0],SevenSegLetters[4]);  //L
        DisplayADigit(dispPorts[1],SevenSegLetters[5]);  //S
        DisplayADigit(dispPorts[2],SevenSegLetters[6]);  //t
        DisplayADigit(dispPorts[3],SevenSegLetters[7]);  //O
      }
      delay(2);
    }
  }
  else if(msg == "error"){  
    // display best available bilingual approximation of "error" 
    //  message on seven-segment display
    unsigned long timer = millis();
    while(millis() - timer < ms){
      DisplayADigit(dispPorts[0],SevenSegLetters[1]);  //E
      DisplayADigit(dispPorts[1],SevenSegLetters[0]);  //r
      DisplayADigit(dispPorts[2],SevenSegLetters[0]);  //r
      DisplayADigit(dispPorts[3],SevenSegNumbers[12]);  //
    }
  }
  else if(msg == "clear"){
    unsigned long timer = millis();
    while(millis() - timer < ms){
      for(int i = 0; i < num_displays; i++){
        DisplayADigit(dispPorts[i],SevenSegNumbers[12]);
      }    
    }
  }
  turnOffDisplay();
}

/*---------------------Interrupt and TSL230R Functions-----------------------*/
void add_pulse() {
  pulse_count++;
} 
//this simple function counts pulses sent from the sensor

void setSensitivity(int sens){    //set sensor sensitivity
  if(sens == LOW_SENSITIVITY){  
    digitalWrite(TSL_S0, LOW);
    digitalWrite(TSL_S1, HIGH);
    sensitivity = LOW_SENSITIVITY;
  }
  else if(sens == MED_SENSITIVITY){
    digitalWrite(TSL_S0, HIGH);
    digitalWrite(TSL_S1, LOW);
    sensitivity = MED_SENSITIVITY;
  }
  else if(sens == HIGH_SENSITIVITY){
    digitalWrite(TSL_S0, HIGH);
    digitalWrite(TSL_S1, HIGH);
    sensitivity = HIGH_SENSITIVITY;
  }
  return;
}


float rawValue(int num_rdgs){
  digitalWrite(IR_LED, HIGH); //turn on light source
  int rep_cnt = 0, b = 0;
  long sum = 0, low = 1000000, high = 0, rd = 0;
  displayForInterval(-1, "cycle_dashes", 1000);
  PCintPort::attachInterrupt(TSL_FREQ, add_pulse, RISING);  
  //turn on frequency-counting function
  delay(200);
  pulse_count = 0; //reset frequency counter
  timer = 0;
  while(rep_cnt < num_rdgs){                    
    //for given number of readings
    if(millis() - timer >= SAMPLING_WINDOW){    
      //once 1000 ms have elapsed
      //normalize frequency by TSL_S2 & TSL_S3 settings
      rd = pulse_count * scale_divider;
      //find highest and lowest readings in the group
      if(rd > high){
        high = rd;
      }                 
      if(rd < low){
        low = rd;
      }
      sum += rd;            //sum the readings
      timer = millis();     //update timer
      rep_cnt++;
      pulse_count = 0;
    }
  }  
  PCintPort::detachInterrupt(TSL_FREQ);        //turn off frequency-counting function
  digitalWrite(IR_LED, LOW);                   //turn off light source
  if(num_rdgs > 3){                  
    // chuck out highest and lowest readings 
    // and average the rest, if there are four or more readings
    sum -= (high + low);
    b = 2;
  }


  float raw_value = float(sum) / float(num_rdgs - b) / div_fact;
  return raw_value;
}


float takeReadings(int num_rdgs){
  float raw_value = rawValue(num_rdgs);
  float ntu_value = -1;
  // get average reading, with highest and lowest discarded
  // for much higher turbidities, code below could easily be expanded 
  // and sensitivity dynamically adjusted
  if(sensitivity == HIGH_SENSITIVITY){  
    // map averaged raw sensor value to NTU
    // using calibration info stored in persistent memory
    if(raw_value > config.y4)       {ntu_value = raw_value * config.m4 + config.b4;}   
    else if(raw_value > config.y3)  {ntu_value = raw_value * config.m3 + config.b3;}
    else if(raw_value > config.y2)  {ntu_value = raw_value * config.m2 + config.b2;}
    else if(raw_value > config.y1)  {ntu_value = raw_value * config.m1 + config.b1;}  
    else                            {ntu_value = raw_value * config.m0 + config.b0;}
    return ntu_value;
  }
  else{return 9999;}
}

String baseNmap(float val){
  // baseNmap encodes turbidity values in a base64 cipher, 
  // to save space if transmitting many values at once
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


/*------------------------------GSM Modem------------------------------------*/
//NOTE: connect the modem to pins 2 (TX), 3 (RX), 7 (RESET), 5V, and GND.
String sendMessage(char* phonenumber, String message){
  sms.beginSMS(phonenumber);  // send the message
  sms.print(message);
  sms.endSMS();
}

void openConnection(){
  notConnected = true;
  while (notConnected) {
    if(gsmAccess.begin(PINNUMBER)==GSM_READY){
      notConnected = false;
    }
    else{
      delay(1000);
    }
  }
}

void closeConnection(){
  while(notConnected==false){
    if(gsmAccess.shutdown()==1){
      notConnected = true;
    }
  }
}

String getMessageText(){
  char c;
  String s;

  // If there are any SMSs available()  
  if(sms.available()){    
    // Read message bytes and print them
    while(c=sms.read()){
      s += c;
    }
    sms.flush();
  }
  return s;
}

void parseMessage(String msg){
  /*  Takes: Text message string of form "PIN#command#arguments"
  */
  String pass, command, arguments;
  int i1 = msg.indexOf("#");
  int i2 = msg.lastIndexOf("#");
  pass = msg.substring(0,i1);
  command = msg.substring(i1+1,i2);
  arguments = msg.substring(i2+1,msg.length());
  msg = "";

  if(pass == config.password){
    if(command=="calibrate"){
      calibrate();
    }
    else if(command=="adjsam"){
          // For inline version: adjust sample rate
          // Not implemented
    }
    else if(command=="confom"){
          // Confirm receipt 
          // send message to <remote_num> with text "<sensor_id> running"
          String confirmation = "unit " + String(config.machine_id) + " is running";
          sendMessage(config.remoteNum, confirmation);
    }
    else if(command=="passalert"){
          // Send message to <user_num> with text from arguments
          sendMessage(config.userpn, arguments);
    }
    else if(command=="changeuser"){
        // Delete old user data
        // EEPROM a struct with 
        // {'userfn': <POC_first_name>, 
        // 'useremail':<POC_email>, 
        // 'userpn': <POC_phone_num>}
        int i1 = arguments.indexOf("#");
        int i2 = arguments.lastIndexOf("#");
        config.userfn = arguments.substring(0,i1);
        config.useremail = arguments.substring(i1+1,i2);
        //String temp = arguments.substring(i2+1,arguments.length());
        arguments.substring(i2+1,arguments.length()).toCharArray(config.userpn,10);
        EEPROM_writeAnything(0, config);
      }
   else if(command=="calibdate"){
      // Retrieve <last_calibration> and send sms to <remote_num>
      sendMessage(config.remoteNum, String(config.last_calibration_timestamp));
    }
    else if(command=="sendrdg"){
      //turn_on_sensor and take_reading
      //send message <remote_num> with <sensor_reading>
      float reading = takeReadings(READ_REPS);   
      String bn, message_text;
      bn = baseNmap(reading);
      message_text = "#un " + String(config.username) + 
                    " #pw " + String(config.password) + 
                    " #reading " + bn;
      sendMessage(config.remoteNum, message_text);
    }
  }
}
/*--------------------------------Calibration--------------------------------*/

void calibrate(){
  float y[10]; //raw readings
  float x[10] = {0.0, 0.2, 0.5, 10, 30, 100, 300, 500, 1200}; //NTU
  float m[5]; // slopes (readings/NTU)

  displayForInterval(0, "ready",50000);

  displayForInterval(x[0], "data",10000);
  // should wait for button here
  y[0] = rawValue(5);

  for(int i = 1; i < 9; i=i+2){
    displayForInterval(x[i], "data",10000);
    // should wait for button here
    y[i] = rawValue(5);
  }

  m[0] = (y[1] - y[0]) / (x[1] - x[0]);

  for(int i = 2; i < 8; i=i+2){
    y[i] = m[i/2-1]*(x[i]-x[i-1]) + y[i-1];
    m[i/2] = (y[i] - y[i-1]) / (x[i] - x[i-1]);
  }


/* Now we have 4 linear equations in point-slope form:
     (y-y0) = m(x-x0),
   where y is the raw reading and x is the NTU value.
   Since normally we want to find NTU from the readings, rearrange:
     x = (1/m)(y-y0) + x0
   in slope-intercept form:
     x = (1/m)y + (x0-y0/m)
    To make calculation simple later, we want 
      ntu_value = raw_value * m + b
    Define a new m = 1/m
    Then b = x0 - m*y0
    These are values we will to save in the config.
*/
  config.foo = 255;                               
  //EEPROMAnything seems to need the struct to start with a integer in [0,255]
  config.machine_id = config.machine_id; //example
  config.last_calibration_timestamp = 1390936721; // TODO: Calculate time here
  config.y0 = y[0];                                  
  config.m0 = 1 / m[0];                              
  config.b0 = x[0] - y[0] / m[0];                              
  config.y1 = y[2];                                  
  config.m1 = 1 / m[1];                              
  config.b1 = x[2] - y[2] / m[1];
  config.y2 = y[4];                                  
  config.m2 = 1 / m[2];                              
  config.b2 = x[4] - y[4] / m[2];
  config.y3 = y[6];                                  
  config.m3 = 1 / m[3];                              
  config.b3 = x[6] - y[6] / m[3];
  config.y4 = y[8];                                  
  config.m4 = 1 / m[4];                              
  config.b4 = x[8] - y[8] / m[4];
  config.remoteNum = config.remoteNum;  //phone number of SMS gateway
  config.selfNum = config.selfNum;  //phone number of your SIM card
  
  EEPROM_writeAnything(0, config);

  displayForInterval(0, "ready",50000);
}
