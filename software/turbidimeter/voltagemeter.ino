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

float getLightMultiplier(){
  //LED operating characteristics: temperature effect on light intensity (from user's manual)
  float led_voltage = 0.0, temp_base = 24, temp_slope = -.00473, temp_multiplier = 1.0;  
  //LED operating characteristics: voltage and current effect on light intensity (from user's manual)
  float led_vi = 1.4, li_base = 1.8125, li_slope = 17.9167, li_multiplier = 0.0;         
  //empirical curve of LED voltage and system voltage for open-source turbidimeter
  led_voltage = .0181 * getVoltageLevel() + 1.3376;                                         
  //calculate light intensity factor of LED due to voltage and current
  li_multiplier = ((led_voltage - led_vi) * li_slope + li_base) / li_base;               
  temperature = readLM35Temperature();
  //calculate light intensity factor of LED due to ambient temperature
  temp_multiplier = 1 + (temperature - temp_base) * temp_slope;                    
  return li_multiplier * temp_multiplier;
} 

float readLM35Temperature(){
  analogReference(INTERNAL);
  delay(200);
  //with 10mV per degree C, and 1100mV for 1024 steps, scaling_factor = 10/(1100/1024) ~ 9.3
  float t =  analogRead(TEMP_PIN) / 9.3;             
  analogReference(DEFAULT);
  delay(200);
  return t;
