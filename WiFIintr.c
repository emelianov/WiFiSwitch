#pragma once
// Interrupt routine handler. They said it should be in separate file to make ICACHE_RAM_ATTR work.
#include <ets_sys.h>
#include <Arduino.h>
#include "mcp3221isr.h"
#include "settings.h"

#define READ_V 1
#define READ_I 2
#define CALC 3

uint8_t mcpA[MCP_COUNT] = {MCP_0, MCP_1, MCP_3};
uint8_t mcp = 0;
uint8_t mcpCalc;
volatile uint16_t mcpDataReady = 0;

volatile bool adcBusy = false;
uint16_t windowV[APPROX_WINDOW];
uint16_t windowI[APPROX_WINDOW];

uint16_t intrAction = READ_V;
int16_t lastFilteredV,filteredV;          //Filtered_ is the raw analog value minus the DC offset
int16_t filteredI;
uint16_t offsetV = ADC_COUNTS>>1;                          //Low-pass filter output
uint16_t offsetI = ADC_COUNTS>>1;                          //Low-pass filter output
int16_t phaseShiftedV;                             //Holds the calibrated phase shifted voltage
uint32_t sqV,sumV,sqI,sumI,instP,sumP;              //sq = squared, sum = Sum, inst = instantaneous
//double V_RATIO;
//double I_RATIO;
volatile int32_t sampleV;                        //sample_ holds the raw analog read value
volatile int32_t sampleI;
//double VCAL     = DEF_VCAL;
//double ICAL     = DEF_ICAL;
int16_t PHASECAL = DEF_PHASECAL;
//uint16_t SupplyVoltage = DEF_SUPPLY;
/*
volatile double realPower[MCP_COUNT],
      apparentPower[MCP_COUNT],
      powerFactor[MCP_COUNT],
      Vrms[MCP_COUNT],
      Irms[MCP_COUNT];
*/
volatile uint16_t samples = 0;

uint16_t ctr = 0;
uint16_t sV[MAX_SAMPLES] = {0};
uint16_t sI[MCP_COUNT][MAX_SAMPLES] = {0};

// Be carefull changing interrupt code
// mcp3221_read() takes ~95microSeconds
// Interrupt must not take above 100-150microSeconds
// Sum of interrupts dutation should not exceed 200-250milliSeconds per second.
// Should not use calls of eny external functions (except ICACHE_RAM_ATTR marked) and float point calculations
// Breaking these limitations leads to controller reset by watchdog ater some time.
void ICACHE_RAM_ATTR timer_isr(){
  if (mcpDataReady) return;
  if (adcBusy) return; // Need add reset of data collection 
  adcBusy = true;
  // Level 1 is used Wi-Fi stack.
  // Level 2 Debug
  // Level 3 NMI (timers?)
  xt_rsil(1);

  if (samples < MAX_SAMPLES) {
    samples++;
    switch (intrAction) {
    case READ_V:
      intrAction = READ_I;
      sampleV = mcp3221_read(MCP_V);
      sV[ctr] = sampleV;
      break;
    case READ_I:
      intrAction = CALC;
      sampleI = mcp3221_read(mcpA[mcp]);
      sI[mcp][ctr] = sampleI;
      ctr++;
      break;
    default:  //CALC
      intrAction = READ_V;
      
      uint32_t _sum = 0;
      uint8_t j = 1;
      uint8_t i;
      for (i = 1; i < APPROX_WINDOW; i++) { 
        if (windowV[i] != 0) {
          _sum += windowV[i];
          j++;
        }
        windowV[i - 1] = windowV[i];
      }
      windowV[APPROX_WINDOW - 1] = sampleV;
      _sum += sampleV;
      sampleV = _sum / j;
      _sum = 0;
      j = 1;
      for (i = 1; i < APPROX_WINDOW; i++) { 
        if (windowI[i] != 0) {
          _sum += windowI[i];
          j++;
        }
        windowI[i - 1] = windowI[i];
      }
      windowI[APPROX_WINDOW - 1] = sampleI;
      _sum += sampleI;
      sampleI = _sum / j;

      lastFilteredV = filteredV;               //Used for delay/phase compensation
      offsetV = offsetV + (sampleV-offsetV) >> 10; // /1024;
      filteredV = sampleV - offsetV;
      offsetI = offsetI + (sampleI-offsetI) >> 10; // /1024;
      filteredI = sampleI - offsetI;
      sqV= filteredV * filteredV;                 //1) square voltage values
      sumV += sqV;                                //2) sum
      sqI = filteredI * filteredI;                //1) square current values
      sumI += sqI;                                //2) sum
      phaseShiftedV = lastFilteredV + PHASECAL * (filteredV - lastFilteredV);
      instP = phaseShiftedV * filteredI;          //Instantaneous Power
      sumP +=instP;                               //Sum
    }
  } else {
/*      V_RATIO = VCAL *((SupplyVoltage / 1000.0) /(ADC_COUNTS));
      Vrms[mcp] = V_RATIO * sqrt(sumV / MAX_SAMPLES);

      I_RATIO = ICAL *((SupplyVoltage / 1000.0) / (ADC_COUNTS));
      Irms[mcp] = I_RATIO * sqrt(sumI / MAX_SAMPLES);

      //Calculation power values
      realPower[mcp] = V_RATIO * I_RATIO * sumP / MAX_SAMPLES;
      apparentPower[mcp] = Vrms[mcp] * Irms[mcp];
      powerFactor[mcp] = realPower[mcp] / apparentPower[mcp];

      sumV = 0;
      sumI = 0;
      sumP = 0;
 */     samples = 0;
      intrAction = READ_V;
      for (uint8_t i = 0; i < APPROX_WINDOW; i++) {
        windowV[i] = 0;
        windowI[i] = 0;
      }
      mcpCalc = mcp;
      mcp++;
      if (mcp >= MCP_COUNT) mcp = 0;
      ctr = 0;
      mcpDataReady++;
  }


 cleanup:
  //cTm = micros() - t;
  adcBusy = false;
}
