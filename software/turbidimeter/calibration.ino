/*--------------------------------Calibration--------------------------------*/

void calibrate(){
  float y[10]; //raw readings
  float x[10] = {0.0, 0.2, 0.5, 10, 30, 100, 300, 500, 1200}; //NTU
  float m[5]; // slopes (readings/NTU)

  displayForInterval(0, "ready",50000);
  Serial.println("Starting calibration.");

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

/* Now we have 5 linear equations in point-slope form:
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
  
  EEPROM_writeAnything(0, config);

  Serial.println("Calibration complete.");
  displayForInterval(0, "ready",50000);
}
