#pragma once
//#include <Arduino.h>
#include "mcp3221isr.h"

#define MCP_V 0x49
#define MCP_1 0x4A
#define MCP_2 0x4B
#define MCP_3 0x4C
#define APOX_WINDOW 3
#define MES_COUNT 2000

// Select constant according to ACS model
//int mVperAmp = 185; //  5A
//int mVperAmp = 100; // 20A
const int mVperAmp = 66;  // 30A

extern uint32_t cTm;

#define CAL_SAMPLES 100

extern "C" void ICACHE_RAM_ATTR timer_isr();
extern volatile int16_t readValue;
extern volatile double realPower,
      apparentPower,
      powerFactor,
      Vrms,
      Irms;
extern double V_RATIO;
extern double I_RATIO;
extern volatile int sampleV;                        //sample_ holds the raw analog read value
extern volatile int sampleI;
extern uint32_t ZERO_V;
extern uint16_t ZERO_I;

uint32_t pri() {
  Serial.printf("Read time: %d, Value: %d/%d/%d\n", cTm, readValue, sampleV, sampleI);
  Serial.printf("V_RATIO: %s, I_RATIO: %s, ZERO_V: %d\n", String(V_RATIO).c_str(), String(I_RATIO).c_str(), ZERO_V);
  Serial.printf("Real: %s, Apparent: %s, PF: %s, Vrms: %s, Irms : %s\n", String(realPower).c_str(), String(apparentPower).c_str(), String(powerFactor).c_str(), String(Vrms).c_str(), String(Irms).c_str());
  //Serial.println(mcp3221_read(0x49));
  //Serial.println(mcp3221_read(0x4A));
  return 5000;
}

uint32_t initA0() {
  //mcp3221_init(400000, 0, 4);
  mcp3221_init(400000, 4, 5);
  ZERO_V = 0;
  for (uint8_t i = 0; i < CAL_SAMPLES; i++) {
    ZERO_V += mcp3221_read(MCP_V);
  }
  ZERO_V /= CAL_SAMPLES;
  taskAdd(pri);
  timer1_disable();
  timer1_attachInterrupt(timer_isr);
  timer1_write(100);  //150
  timer1_enable(TIM_DIV256, TIM_EDGE, TIM_LOOP);
  return RUN_DELETE;
}

float current() {
  return amps;
}
 
uint32_t queryA0() { 
  /*
  double Voltage = 0;
  double VRMS = 0;
   // Subtract min from max
  Voltage =  ((maxValue - minValue) * 5.0)/1024.0;
  VRMS = (Voltage/2.0) *0.707;
  amps = (VRMS * 1000) / mVperAmp - 0.09;
  amps = (VRMS * 100) / mVperAmp - 0.09;
  if (amps < 0) {
    amps = 0.0;
  }
  */
  amps = readValue;
  return A0_DELAY;
}
