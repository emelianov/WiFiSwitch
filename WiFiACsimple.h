#pragma once
//No calibration is needed

// Select constant according to ACS model
//int mVperAmp = 185; //  5A
//int mVperAmp = 100; // 20A
const int mVperAmp = 66;  // 30A

uint32_t initA0() {
  return RUN_DELETE;
}

float current() {
  return amps;
}
 
uint32_t queryA0() { 
  double Voltage = 0;
  double VRMS = 0;
  int readValue;             //value read from the sensor
  int maxValue = 0;          // store max value here
  int minValue = 1024;          // store min value here
  
   uint32_t start_time = millis();
   while((millis()-start_time) < 100) //sample for 0.1 Sec
   {
       readValue = analogRead(A0);
       // see if you have a new maxValue
       if (readValue > maxValue) 
       {
           /*record the maximum sensor value*/
           maxValue = readValue;
       }
       if (readValue < minValue) 
       {
           /*record the maximum sensor value*/
           minValue = readValue;
       }
   }
   
   // Subtract min from max
  Voltage =  ((maxValue - minValue) * 5.0)/1024.0;
  VRMS = (Voltage/2.0) *0.707;
  amps = (VRMS * 1000) / mVperAmp - 0.09;
  if (amps < 0) {
    amps = 0.0;
  }
  return A0_DELAY;
}

