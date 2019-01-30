#pragma once
/*
  Based on following library:
  https://github.com/openenergymonitor/EmonLib
  
  Emon.h - Library for openenergymonitor
  Created by Trystan Lea, April 27 2010
  GNU GPL
  modified to use up to 12 bits ADC resolution (ex. Arduino Due)
  by boredman@boredomprojects.net 26.12.2013
  Low Pass filter for offset removal replaces HP filter 1/1/2015 - RW
*/

#include "mcp3221.h"

#define HISTORY 30

#define ADC_COUNTS (1 << ADC_BITS)

typedef struct power {
  float realPower;
  float apparentPower;
  float powerFactor;
  float Vrms;
  float Irms;
  float Vcc;
} power;

power history[HISTORY][MCP_COUNT];
uint16_t h = 1;
uint16_t l = 0;

uint8_t ch = 0;
uint8_t mcp[MCP_COUNT] = {MCP_0, MCP_1, MCP_3};

uint32_t initA0() {
  mcp3221_init(400000, SDA, SCL);
  WDEBUG("MCPs V: %d, I: %d\n", mcp3221_read(MCP_V), mcp3221_read(mcp[0]));
  //taskAdd(queryA0);
  return RUN_DELETE;
}


    long readVcc();
    //Calibration coefficients
    //These need to be set in order to obtain accurate results
    float VCAL = DEF_VCAL;
    float ICAL = DEF_ICAL;
    float PHASECAL = DEF_PHASECAL;

    //--------------------------------------------------------------------------------------
    // Variable declaration for emon_calc procedure
    //--------------------------------------------------------------------------------------
 
uint32_t queryA0() { 
  int16_t* fV;
  int16_t* fI;
  int SupplyVoltage=DEF_SUPPLY;
  //int SupplyVoltage = readVcc();
  const uint16_t timeout = 300; // Exact count of periods for 50 and 60Hz both
    int sampleV;                        //sample_ holds the raw analog read value
    int sampleI;

    float lastFilteredV,filteredV;          //Filtered_ is the raw analog value minus the DC offset
    float filteredI;
    float offsetV;                          //Low-pass filter output
    float offsetI;                          //Low-pass filter output

    float phaseShiftedV;                             //Holds the calibrated phase shifted voltage.

    float sqV,sumV,sqI,sumI,instP,sumP;              //sq = squared, sum = Sum, inst = instantaneous

    int startV;                                       //Instantaneous voltage at start of sample window.

    boolean lastVCross, checkVCross;                  //Used to measure number of times threshold is crossed.


  unsigned int crossCount = 0;                             //Used to measure number of times threshold is crossed.
  unsigned int numberOfSamples = 0;                        //This is now incremented
  #define F_COUNT (sizeof(data) / sizeof(uint16_t) / 2)
  fV = (int16_t*)data;
  fI = (int16_t*)(data + F_COUNT * sizeof(uint16_t));
  
  // 1) As we using timeout that fits exact number of wave periods no need to find zerr-crossing as in original algorythm
  //-------------------------------------------------------------------------------------------------------------------------
  // 2) Main measurement loop
  //-------------------------------------------------------------------------------------------------------------------------
  unsigned long start = millis();    //millis()-start makes sure it doesnt get stuck in the loop if there is an error.
  
  while ((millis() - start) < timeout && numberOfSamples < F_COUNT) // control numberOfSmples just to escape overflow
  {
    //-----------------------------------------------------------------------------
    // A) Read in raw voltage and current samples
    //-----------------------------------------------------------------------------
    fV[numberOfSamples] = mcp3221_read(MCP_V);                 //Read in raw voltage signal
    fI[numberOfSamples] = mcp3221_read(mcp[ch]);               //Read in raw current signal
    numberOfSamples++;                                         //Count number of times looped.
  }
  // Calculate and remove offset
  offsetV = 0;
  offsetI = 0;
  for (uint16_t i = 0; i < numberOfSamples; i++) {
    offsetV += fV[i];
    offsetI += fI[i];
  }
  offsetV /= (numberOfSamples);
  offsetI /= (numberOfSamples);
  for (uint16_t i = 0; i < numberOfSamples; i++) {
    fV[i] -= offsetV;
    fI[i] -= offsetI;
  }

  // Approximate dataset
  for (uint16_t i = 2; i < numberOfSamples - 2; i++) {
    fV[i] = (fV[i - 2] + fV[i - 1] + fV[i] + fV[i  + 1] + fV[i + 2])/5;
    fI[i] = (fI[i - 2] + fI[i - 1] + fI[i] + fI[i  + 1] + fI[i + 2])/5;
  }
  // Second approximation pass
  for (uint16_t i = 2; i < numberOfSamples - 2; i++) {
    fV[i] = (fV[i - 2] + fV[i - 1] + fV[i] + fV[i  + 1] + fV[i + 2])/5;
    fI[i] = (fI[i - 2] + fI[i - 1] + fI[i] + fI[i  + 1] + fI[i + 2])/5;
  }
  
  filteredV = fV[2];
  for (uint16_t i = 2; i < numberOfSamples - 2; i++) {
    sampleV = fV[i];
    sampleI = fI[i];
    lastFilteredV = filteredV;               //Used for delay/phase compensation
    //-----------------------------------------------------------------------------
    // B) Apply digital low pass filters to extract the 2.5 V or 1.65 V dc offset,
    //     then subtract this - signal is now centred on 0 counts.
    //-----------------------------------------------------------------------------
   // offsetV = offsetV + ((sampleV-offsetV)/4096);
    filteredV = (abs(sampleV) > NOISE_FLOOR)?sampleV:0;// - offsetV;
   // offsetI = offsetI + ((sampleI-offsetI)/4096);
    filteredI = (abs(sampleI) > NOISE_FLOOR)?sampleI:0;// - offsetI;

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

  }

  //-------------------------------------------------------------------------------------------------------------------------
  // 3) Post loop calculations
  //-------------------------------------------------------------------------------------------------------------------------
  //Calculation of the root of the mean of the voltage and current squared (rms)
  //Calibration coefficients applied.
  history[h][ch].Vcc = ESP.getVcc();
  float V_RATIO = VCAL *((SupplyVoltage/1000.0) / (ADC_COUNTS));
  history[h][ch].Vrms = V_RATIO * sqrt(sumV / (numberOfSamples - 4));

  float I_RATIO = ICAL *((SupplyVoltage/1000.0) / (ADC_COUNTS));
  history[h][ch].Irms = I_RATIO * sqrt(sumI / (numberOfSamples - 4));

  //Calculation power values
  history[h][ch].realPower = V_RATIO * I_RATIO * sumP / (numberOfSamples - 4);
  history[h][ch].apparentPower = history[h][ch].Vrms * history[h][ch].Irms;
  history[h][ch].powerFactor = history[h][ch].realPower / history[h][ch].apparentPower;

  //Reset accumulators
  sumV = 0;
  sumI = 0;
  sumP = 0;
//--------------------------------------------------------------------------------------
 #ifdef WFS_DEBUG
  String sVrms = String(history[h][ch].Vrms);
  String sIrms = String(history[h][ch].Irms);
  String sRealPower = String(history[h][ch].realPower);
  String sPowerFactor = String(history[h][ch].powerFactor);
  String sApparentPower = String( history[h][ch].apparentPower);
  WDEBUG("Ch: %d, Vrms: %s, Irms: %s, realPower: %s, powerFactor: %s, Samples count: %d, Mem: %d\n",
        ch, sVrms.c_str(), sIrms.c_str(), sRealPower.c_str(), sPowerFactor.c_str(), numberOfSamples, ESP.getFreeHeap());
 #endif
  ch++;
  if (ch >= MCP_COUNT) {
    ch = 0;
    h++;
    if (h >= HISTORY) {
      h = 0;
    }
    l++;
    if (l >= HISTORY) {
      l = 0;
    }
  }
  return A0_DELAY;
}
