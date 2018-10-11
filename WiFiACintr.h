#pragma once
#include <Wire.h>

//No calibration is needed



// Select constant according to ACS model
//int mVperAmp = 185; //  5A
//int mVperAmp = 100; // 20A
const int mVperAmp = 66;  // 30A

extern int maxValue;
extern int minValue;
extern uint32_t cTm;

extern "C" void ICACHE_RAM_ATTR timer_isr();

uint32_t pri() {
  Serial.println(cTm);
  return 5000;
}

uint32_t initA0() {
  //return RUN_DELETE;
  timer1_disable();
  timer1_attachInterrupt(timer_isr);
  timer1_write(150);
  timer1_enable(TIM_DIV256, TIM_EDGE, TIM_LOOP);
  taskAdd(pri);
  return RUN_DELETE;
}

float current() {
  return amps;
}
 
uint32_t queryA0() { 
  double Voltage = 0;
  double VRMS = 0;
   // Subtract min from max
  Voltage =  ((maxValue - minValue) * 5.0)/1024.0;
  VRMS = (Voltage/2.0) *0.707;
  amps = (VRMS * 1000) / mVperAmp - 0.09;
  if (amps < 0) {
    amps = 0.0;
  }
  return A0_DELAY;
}

