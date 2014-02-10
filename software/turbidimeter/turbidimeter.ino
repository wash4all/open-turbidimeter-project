/*----------------OPEN TURBIDIMETER PROJECT---------------*/
/*--------------------- ARDUINO CODE ---------------------*/
/*
Developed by WASH4All, 2014. See wash4all.org for further
information about the project.
*/
/*-------------------------LEGAL--------------------------*/

/* This device and its components are licensed under a
Creative Commons Attribution-NonCommercial-ShareAlike 4.0 
Unported  (CC BY-NC-SA 4.0) license. In summary,
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
#include "EEPROMAnything.h"

/*ROLES:	each of which can have an associated sms password
developer  -- for changing technical specs, checking in on usage stats, etc.
admin	-- e.g. NGO that purchased the device
user	-- POC for device
group	-- group of users, e.g. water board or operators
public	-- general public
*/

/*EXAMPLES:
<group_pw>#adjsam#3600
public#confon
<group_pw>#passalert#your turbidimeter is sending weird readings
<user_pw>#changeuser#<username> <useremail> <userphone>
public#calibrate
public#sendreading
*/

void setup(){
  Serial.begin(9600);
  //Set up modem functions
  //put modem to sleep
}

void loop(){
  //turn on modem
  String s = getMessageText(); //say output is "<password>#adjsam#3600"
  parseMessage(s);
  //turn off modem
  delay(900000);
}

void parseMessage(String s){
  String s1, s2, s3;
  int i1 = s.indexOf("#");
  int i2 = s.lastIndexOf("#");
  s1 = s.substring(0,i1);
  s2 = s.substring(i1+1,i2);
  s3 = s.substring(i2+1,s.length());
  s = "";
  if(s1 == "<password>"){
    Serial.println("passes");
    if(s2 == "adjsam"){
      Serial.println("adjust sampling rate to " + s3 + " seconds per sample...");
    }else if(s2 == "confon"){
      Serial.println("send a message confirming receipt of message...");
      //send message to <remote_num> with text "<sensor_id> running"
    }else if(s2 == "passalert"){
      Serial.println("pass the contents of this message on to the ...");
      //send message to <user_num> with text (words[i] for i > 2)
    }else if(s2 == "changeuser"){
      Serial.println("change the stored user name and number...");
      //delete old user struct
      //EEPROM a struct with {'userfn': <POC_first_name>, 'useremail':<POC_email>, 'userpn': <POC_phone_num>}
    }else if(s2 == "calibdate"){
      Serial.println("send a message with the date of the last device calibration...");
      //retrieve <last_calibration_date> and send sms to <remote_num>
    }else if(s2 == "sendrdg"){
      Serial.println("send an immediate turbidity reading...");
      //turn_on_sensor and take_reading
      //send message <remote_num> with <sensor_reading>
    }else{
      Serial.println("command unrecognized");
      //do nothing
    }
  }
}
