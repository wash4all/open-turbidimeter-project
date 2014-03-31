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