#pragma once
#include <ets_sys.h>
#include <Arduino.h>
#include "mcp3221isr.h"
//#include "WiFiACintr.h"

#define MCP_V 0x49
#define MCP_1 0x4A

#define ADC_COUNTS  (1<<ADC_BITS)

//extern volatile uint16_t samples4A[MES_COUNT>>1];
//extern volatile uint16_t samples49[MES_COUNT>>1];
//extern bool fillSamples;

  #define READ_V 1
  #define READ_I 2
  #define CALC 3

uint32_t ZERO_V;
volatile uint32_t cTm = 0;
volatile int16_t readValue;
volatile bool adcBusy = false;
//uint8_t mcpI[MCP_COUNT] = {MCP_1, MCP_1, MCP_1};
#define APPROX_WINDOW 5
uint16_t windowV[APPROX_WINDOW];
uint16_t windowI[APPROX_WINDOW];

double VCAL     = 2732.91 * 2.25;  //234.26;
double ICAL     = 111.1;
double PHASECAL = 1.7;
uint16_t SupplyVoltage=3300;
uint16_t crossings = 20;

uint16_t intrAction = READ_V;
boolean st=false;
uint16_t crossCount = 0;                             //Used to measure number of times threshold is crossed.
uint16_t numberOfSamples = 0;                        //This is now incremented
double lastFilteredV,filteredV;          //Filtered_ is the raw analog value minus the DC offset
double filteredI;
double offsetV;                          //Low-pass filter output
double offsetI;                          //Low-pass filter output
double phaseShiftedV;                             //Holds the calibrated phase shifted voltage
double sqV,sumV,sqI,sumI,instP,sumP;              //sq = squared, sum = Sum, inst = instantaneous
//int startV;                                       //Instantaneous voltage at start of sample window.
boolean lastVCross, checkVCross;                  //Used to measure number of times threshold is crossed.
double V_RATIO;
double I_RATIO;
volatile int sampleV;                        //sample_ holds the raw analog read value
volatile int sampleI;

volatile double realPower,
      apparentPower,
      powerFactor,
      Vrms,
      Irms;


void ICACHE_RAM_ATTR timer_isr(){
  if (adcBusy) return;
  uint32_t t = micros();
  adcBusy = true;
  // Level 1 is used Wi-Fi stack.
  // Level 2 Debug
  // Level 3 NMI (timers?)
  xt_rsil(1);
  //readValue = mcp3221_read(0x49);

  if (!st)
  {
    readValue = mcp3221_read(MCP_V);                    //using the voltage waveform
    //if ((readValue < (ADC_COUNTS*0.55)) && (readValue > (ADC_COUNTS*0.45))) {   //check its within range
    if ((readValue > ZERO_V * 0.99) && (readValue < ZERO_V * 1.01)) {   //check its within range
    //if (true) {
      st = true;
      crossCount = 0;
      numberOfSamples = 0;
    }
    goto cleanup;
  }
  if ((crossCount < crossings))
  {
    switch (intrAction) {
    case READ_V:
      sampleV = mcp3221_read(MCP_V);
     {
      uint32_t _sum = 0;
      uint8_t j = 1;
      for (uint8_t i = 1; i < APPROX_WINDOW; i++) { 
        if (windowV[i] != 0) {
          _sum += windowV[i];
          j++;
        }
        windowV[i - 1] = windowV[i];
      }
      windowV[APPROX_WINDOW - 1] = sampleV;
      _sum += sampleV;
      sampleV = _sum / j;
    }
      intrAction = READ_I;
    break;
    case READ_I:
      sampleI = mcp3221_read(MCP_1);
    {
      uint32_t _sum = 0;
      uint8_t j = 1;
      for (uint8_t i = 1; i < APPROX_WINDOW; i++) { 
        if (windowI[i] != 0) {
          _sum += windowI[i];
          j++;
        }
        windowI[i - 1] = windowI[i];
      }
      windowI[APPROX_WINDOW - 1] = sampleI;
      _sum += sampleV;
      sampleI = _sum / j;
    }
      intrAction = CALC;
    break;
    default:  //CALC
      intrAction = READ_V;

    numberOfSamples++;                       //Count number of times looped.
    lastFilteredV = filteredV;               //Used for delay/phase compensation

    //-----------------------------------------------------------------------------
    // A) Read in raw voltage and current samples
    //-----------------------------------------------------------------------------
    //sampleV = mcp3221_read(MCP_V);                 //Read in raw voltage signal
    //sampleI = mcp3221_read(MCP_1);                 //Read in raw current signal

    //-----------------------------------------------------------------------------
    // B) Apply digital low pass filters to extract the 2.5 V or 1.65 V dc offset,
    //     then subtract this - signal is now centred on 0 counts.
    //-----------------------------------------------------------------------------
    offsetV = offsetV + ((sampleV-offsetV)/1024);
    filteredV = sampleV - offsetV;
    offsetI = offsetI + ((sampleI-offsetI)/1024);
    filteredI = sampleI - offsetI;

    //-----------------------------------------------------------------------------
    // C) Root-mean-square method voltage
    //-----------------------------------------------------------------------------
    sqV= filteredV * filteredV;                 //1) square voltage values
    sumV += sqV;                                //2) sum

    //-----------------------------------------------------------------------------
    // D) Root-mean-square method current
    //-----------------------------------------------------------------------------
    sqI = filteredI * filteredI;                //1) square current values
    sumI += sqI;                                //2) sum

    //-----------------------------------------------------------------------------
    // E) Phase calibration
    //-----------------------------------------------------------------------------
    phaseShiftedV = lastFilteredV + PHASECAL * (filteredV - lastFilteredV);

    //-----------------------------------------------------------------------------
    // F) Instantaneous power calc
    //-----------------------------------------------------------------------------
    instP = phaseShiftedV * filteredI;          //Instantaneous Power
    sumP +=instP;                               //Sum

    //-----------------------------------------------------------------------------
    // G) Find the number of times the voltage has crossed the initial voltage
    //    - every 2 crosses we will have sampled 1 wavelength
    //    - so this method allows us to sample an integer number of half wavelengths which increases accuracy
    //-----------------------------------------------------------------------------
    lastVCross = checkVCross;
    if (sampleV > readValue) checkVCross = true;
                     else checkVCross = false;
    if (numberOfSamples==1) lastVCross = checkVCross;

    if (lastVCross != checkVCross) crossCount++;
    }
  } else {
      V_RATIO = VCAL *((SupplyVoltage/1000.0) / (ADC_COUNTS));
      Vrms = V_RATIO * sqrt(sumV / numberOfSamples);

      I_RATIO = ICAL *((SupplyVoltage/1000.0) / (ADC_COUNTS));
      Irms = I_RATIO * sqrt(sumI / numberOfSamples);

      //Calculation power values
      realPower = V_RATIO * I_RATIO * sumP / numberOfSamples;
      apparentPower = Vrms * Irms;
      powerFactor=realPower / apparentPower;

      //Reset accumulators
      sumV = 0;
      sumI = 0;
      sumP = 0;
      st = false;
      for (uint8_t i = 0; i < APPROX_WINDOW; i++) {
        windowV[i] = 0;
        windowI[i] = 0;
      }
  }


 cleanup:
  cTm = micros() - t;
  adcBusy = false;
}
