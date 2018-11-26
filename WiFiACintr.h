#pragma once
// AC Power meter.
// MCP3221 timer interrupt version

#include "mcp3221isr.h"
#include "settings.h"

extern "C" void ICACHE_RAM_ATTR timer_isr();

//extern volatile int32_t sampleV;                        //sample_ holds the raw analog read value
//extern volatile int32_t sampleI;
//extern double offsetV,offsetI;          //Filtered_ is the raw analog value minus the DC offset
//extern double filteredI;

//extern uint32_t sqV,sumV,sqI,sumI,instP,sumP;              //sq = squared, sum = Sum, inst = instantaneous
//double V_RATIO;
//double I_RATIO;
double VCAL     = DEF_VCAL;
double ICAL     = DEF_ICAL;
uint16_t SupplyVoltage = DEF_SUPPLY;

double realPower[MCP_COUNT],
      apparentPower[MCP_COUNT],
      powerFactor[MCP_COUNT],
      Vrms[MCP_COUNT],
      Irms[MCP_COUNT];

extern uint8_t mcpCalc;
extern volatile uint16_t mcpDataReady;
extern int16_t sV[MAX_SAMPLES];
extern int16_t sI[MAX_SAMPLES];
extern int16_t sW[MAX_SAMPLES];

uint32_t pri() {
  SupplyVoltage = ESP.getVcc();
 #ifdef WFS_DEBUG
  //Serial.printf("%d/%d\n", sampleV, sampleI);
  //Serial.printf("V_RATIO: %s, I_RATIO: %s\n", String(V_RATIO).c_str(), String(I_RATIO).c_str());
  //Serial.printf("offesetV: %s, offsetI: %s, VCC: %d\n", String(offsetV).c_str(), String(offsetI).c_str(), SupplyVoltage);
  Serial.printf("Real: %s, Apparent: %s, PF: %s, Vrms: %s, Irms : %s, Mem: %d\n", String(realPower[0]).c_str(), String(apparentPower[0]).c_str(), String(powerFactor[0]).c_str(), String(Vrms[0]).c_str(), String(Irms[0]).c_str(), ESP.getFreeHeap());
 #endif
  return 5000;
}

#define NZEROS 4
#define NPOLES 4
#define GAIN   2.982294417e+02

uint32_t powerCalc() {
  SupplyVoltage = ESP.getVcc();
  // Calc bias
  int32_t sum = 0;
  uint16_t samples = 0;
  for (uint16_t i = 0; i < MAX_SAMPLES; i++){
    if (sV[i] <= 1) continue;
    sum += sV[i];
    samples++;
  }
  sum /= (samples);
  // Normalize V
  for (uint16_t i = 0; i < MAX_SAMPLES; i++){
    if (sV[i] <= 1) sV[i] = sum;
    sV[i] -= sum;
  }
  // Calc bias
  sum = 0;
  samples = 0;
  for (uint16_t i = 0; i < MAX_SAMPLES; i++){
    if (sI[i] <= 1) continue;
    sum += sI[i];
    samples++;
  }
  sum /= samples;
  yield();
  // Normalize I + 60Hz band filter
  float xv[NZEROS+1], yv[NPOLES+1];
  for (uint16_t i = 0; i < MAX_SAMPLES; i++){
    xv[0] = xv[1]; xv[1] = xv[2]; xv[2] = xv[3]; xv[3] = xv[4];
    if (sI[i] > 1) 
      xv[4] = (sI[i] - sum) / GAIN;
    else
      xv[4] = 0;
    yv[0] = yv[1]; yv[1] = yv[2]; yv[2] = yv[3]; yv[3] = yv[4]; 
    yv[4] =   (xv[0] + xv[4]) - 2 * xv[2]
                     + ( -0.8430617666 * yv[0]) + (  3.2937365649 * yv[1])
                     + ( -5.0531136780 * yv[2]) + (  3.5877903540 * yv[3]);
    sI[i] = yv[4];
    yield();
  }
  // yield();
  double sumV = 0;
  double sumI = 0;
  double sumP = 0;
  double sampleV;
  double sampleI;
  double sqV,sqI,instP;
  // Calculate RMS and Instant power sums
  // Some magic: skip first 1/2 of samples to use flat data part afer filter
  for (uint16_t i = MAX_SAMPLES / 2; i < MAX_SAMPLES; i++) {
    sampleI = sI[i];
    sampleV = sV[i];
    sqV = sampleV * sampleV;
    sumV += sqV;
    sqI = sampleI * sampleI;
    sumI += sqI;
    instP = sampleV * sampleI;
    sumP +=instP;
    yield();
  }
  double V_RATIO = VCAL *((SupplyVoltage / 1000.0) /(ADC_COUNTS));
  //Vrms[mcp] = V_RATIO * sqrt(sumV / MAX_SAMPLES);
  // MAX_SAMPLES / 2.0 because of 1/2 frucation
  Vrms[mcpCalc] = V_RATIO * sqrt(sumV / (MAX_SAMPLES / 2.0));

  double I_RATIO = ICAL *((SupplyVoltage / 1000.0) / (ADC_COUNTS));
  //Irms[mcp] = I_RATIO * sqrt(sumI / MAX_SAMPLES);
  //Serial.printf("SQRT(%s)=%s", String(sumI / (cnt * 1.5)).c_str(), String(sqrt(sumI / (cnt * 1.5))).c_str());
  // MAX_SAMPLES / 2.0 because of 1/2 frucation
  Irms[mcpCalc] = I_RATIO * sqrt(sumI / (MAX_SAMPLES / 2.0));

  //Calculation power values
  realPower[mcpCalc] = V_RATIO * I_RATIO * sumP / (MAX_SAMPLES / 2.0);
  apparentPower[mcpCalc] = Vrms[mcpCalc] * Irms[mcpCalc];
  powerFactor[mcpCalc] = realPower[mcpCalc] / apparentPower[mcpCalc];
  if (Irms[mcpCalc]*110 > 12500) {
    for (uint8_t i = 0; i < MAX_SAMPLES; i++) {
      sW[i] = sI[i];
    }
  }
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
  taskAddWithSemaphore(powerCalc, (uint16_t*)&mcpDataReady);
  // Set timer interrupt frequency to 3150Hz
  timer1_disable();
  timer1_attachInterrupt(timer_isr);
  timer1_write(150);  // 2082Hz
  timer1_enable(TIM_DIV256, TIM_EDGE, TIM_LOOP);
  
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
