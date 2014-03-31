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

