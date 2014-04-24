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
        int i1 = arguments.indexOf("!");
        int i2 = arguments.lastIndexOf("!");
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
