/*--------------------------OPEN TURBIDIMETER PROJECT------------------------*/
/*------------------------------- ARDUINO CODE ------------------------------*/
/*---Developed by WASH4All, 2014. See wash4all.org for further information.--*/

// Flags
const boolean debug=false;  // to reset memory to default config -> true
const boolean using_modem=false; // if not using a GSM modem, change to false
  
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
#define VERSION_BYLINE "Open Turb Prj\nBIOS v1.11\n2014-04-08\n"
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
   If that number is followed by a plus () sign, the display is common cathode.
*/

#define common_cathode_display 1 //if using a common-anode seven segment display, change to false (0)
#if common_cathode_display
#define SEVEN_SEG_NUMBERS {B11000000,  B11111001,  B10100100,  B10110000, B10011001,  B10010010,  B10000010,  B11111000, B10000000,  B10010000,  B01111111,  B10111111, B11111111}
#define SEVEN_SEG_LETTERS {B10101111,  B10000110,  B10100001,  B10010001, B11000111,  B10010010,  B10000111,  B11000000}
#else
#define SEVEN_SEG_NUMBERS {B00111111,  B00000110,  B01011011,  B01001111, B01100110,  B01101101,  B01111101,  B00000111, B01111111,  B01101111,  B10000000,  B01000000, B00000000}
#define SEVEN_SEG_LETTERS {B01010000,  B01111001,  B01011110,  B01101110, B00111000,  B01101101,  B01111000,  B00111111}
#endif

// Gauge voltage by comparison to ATMega328P's internal 1.1v
// R1 and R2 form a voltage divider, with V_out = V_in * R2 / (R1  R2)
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
char serialInput;

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
  // y is the lower bound, m is the slope, b is the the y-intercept (y=mxb)
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
  // Set up serial commication
  Serial.begin(9600);
  Serial.println(VERSION_BYLINE);
  Serial.println("Press r to read, c to calibrate.");
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
  for(int i = 0; i < num_displays; i){
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
    config.machine_id = 11111111;
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
    config.remoteNum = "14109278905";
    config.selfNum = "1410*******";
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
  // Given there is enough battery power for the  sensor,
  // - read sensor value for given number of times,
  // - display the resulting value for 4000 milliseconds,
  // - clear out register pins, for a clean display next 
  //   time device is powered off/on.
  // - build a text message and transmit,
  // - check for and parse incoming messages,
  //   if using_modem flag is set to true.  
  // - check for incoming serial commands and process them.
  boolean sufficient_battery = true;
  float reading;
  if(sufficient_battery){
    // SERIAL CONTROL OPTIONS
    serialInput = 'x';
    if (Serial.available() > 0) {
      serialInput = Serial.read(); // get one-byte command option
      Serial.println((char)serialInput); // tell the user what they pressed
    }    
    // check for button press event (0 = pressed)
    bpress = analogRead(BPIN); 
    if (bpress >= BUTTON_EVENT_VALUE || serialInput == 'r'){
      bpressed = true;
      divisionFactor_TSL230R();                
      // read, but discard first reading
      div_fact = divisionFactor_TSL230R();     
      // take another reading of voltage divider
      if(div_fact < 0){
        Serial.println("Battery low...");
        sufficient_battery = false;
      }     
      else{ 
        Serial.println("Taking reading...");        
        reading = takeReadings(READ_REPS);
        Serial.print(reading); 
        Serial.print(" NTU\n");
        displayForInterval(reading, "data",4000);            
        displayForInterval(-1, "clear", 100);

        // GSM CONTROL OPTIONS
        if(using_modem){                                     
          int msg_len = 140;
          char txtMsg[msg_len];
          String bn, message_text;
          bn = baseNmap(reading);
          message_text = "#un " + (String)config.username +
                        " #pw " + (String)config.password +
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
    else if(serialInput == 'c'){
      calibrate();
    }
    else if(serialInput != 'x'){
      Serial.println("Invalid input. Press r to read, c to calibrate.");
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
