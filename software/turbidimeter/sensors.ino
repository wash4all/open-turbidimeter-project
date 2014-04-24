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

  float if_factor = getLightMultiplier();
  float raw_value = float(sum) / float(num_rdgs - b) / div_fact / if_factor;
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
