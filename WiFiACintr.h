#pragma once
// AC Power meter.
// MCP3221 from timer interrupt version

#include "mcp3221isr.h"
#include "settings.h"

// Select constant according to ACS model
//int mVperAmp = 185; //  5A
//int mVperAmp = 100; // 20A
//const int mVperAmp = 66;  // 30A

//extern uint16_t mcpDataReady;

extern "C" void ICACHE_RAM_ATTR timer_isr();
//extern volatile int16_t readValue;

extern volatile int32_t sampleV;                        //sample_ holds the raw analog read value
extern volatile int32_t sampleI;
extern double offsetV,offsetI;          //Filtered_ is the raw analog value minus the DC offset
extern double filteredI;

extern uint32_t sqV,sumV,sqI,sumI,instP,sumP;              //sq = squared, sum = Sum, inst = instantaneous
double V_RATIO;
double I_RATIO;
//volatile int32_t sampleV;                        //sample_ holds the raw analog read value
//volatile int32_t sampleI;
double VCAL     = DEF_VCAL;
double ICAL     = DEF_ICAL;
//int16_t PHASECAL = DEF_PHASECAL;
uint16_t SupplyVoltage = DEF_SUPPLY;

double realPower[MCP_COUNT],
      apparentPower[MCP_COUNT],
      powerFactor[MCP_COUNT],
      Vrms[MCP_COUNT],
      Irms[MCP_COUNT];

extern uint8_t mcpCalc;
extern volatile uint16_t mcpDataReady;

uint32_t pri() {
  SupplyVoltage = ESP.getVcc();
 #ifdef WFS_DEBUG
  Serial.printf("%d/%d\n", sampleV, sampleI);
  Serial.printf("V_RATIO: %s, I_RATIO: %s\n", String(V_RATIO).c_str(), String(I_RATIO).c_str());
  Serial.printf("offesetV: %s, offsetI: %s, VCC: %d\n", String(offsetV).c_str(), String(offsetI).c_str(), SupplyVoltage);
  Serial.printf("Real: %s, Apparent: %s, PF: %s, Vrms: %s, Irms : %s\n", String(realPower[0]).c_str(), String(apparentPower[0]).c_str(), String(powerFactor[0]).c_str(), String(Vrms[0]).c_str(), String(Irms[0]).c_str());
 #endif
  return 5000;
}

uint32_t powerCalc() {
      V_RATIO = VCAL *((SupplyVoltage / 1000.0) /(ADC_COUNTS));
      Vrms[mcpCalc] = V_RATIO * sqrt(sumV / MAX_SAMPLES);

      I_RATIO = ICAL *((SupplyVoltage / 1000.0) / (ADC_COUNTS));
      Irms[mcpCalc] = I_RATIO * sqrt(sumI / MAX_SAMPLES);

      //Calculation power values
      realPower[mcpCalc] = V_RATIO * I_RATIO * sumP / MAX_SAMPLES;
      apparentPower[mcpCalc] = Vrms[mcpCalc] * Irms[mcpCalc];
      powerFactor[mcpCalc] = realPower[mcpCalc] / apparentPower[mcpCalc];

      sumV = 0;
      sumI = 0;
      sumP = 0;
      return RUN_NEVER;
}

uint32_t initA0() {
 #ifdef WFS_DEBUG
  mcp3221_init(400000, SDA, SCL);
  Serial.println(mcp3221_read(0x49));
  Serial.println(mcp3221_read(MCP_V));
 #else
  mcp3221_init(400000, SDA, SCL);
 #endif
  taskAdd(pri);
  timer1_disable();
  timer1_attachInterrupt(timer_isr);
  timer1_write(100);  //150
  timer1_enable(TIM_DIV256, TIM_EDGE, TIM_LOOP);
  taskAddWithSemaphore(powerCalc, (uint16_t*)&mcpDataReady);
  return RUN_DELETE;
}

float current() {
  //return realPower;
  return 0;
}
 
uint32_t queryA0() { 
  //amps = realPower;
  return A0_DELAY;
}
