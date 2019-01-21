#pragma once
#include "mcp3221.h"

double realPowers[MCP_COUNT],
      apparentPowers[MCP_COUNT],
      powerFactors[MCP_COUNT],
      Vrmss[MCP_COUNT],
      Irmss[MCP_COUNT];

uint8_t ch = 0;

uint32_t initA0() {
 #ifdef WFS_DEBUG
  mcp3221_init(400000, SDA, SCL);
  Serial.println(mcp3221_read(MCP_0));
  Serial.println(mcp3221_read(MCP_V));
 #else
  mcp3221_init(400000, SDA, SCL);
 #endif

  //taskAdd(queryA0);
  return RUN_DELETE;
}


    long readVcc();
    //Useful value variables
    double realPower,
      apparentPower,
      powerFactor,
      Vrms,
      Irms;


    //Set Voltage and current input pins
    unsigned int inPinV;
    unsigned int inPinI;
    //Calibration coefficients
    //These need to be set in order to obtain accurate results
    double VCAL = 50000.0;
    double ICAL = 27.0;
    double PHASECAL = 0;

    //--------------------------------------------------------------------------------------
    // Variable declaration for emon_calc procedure
    //--------------------------------------------------------------------------------------
    int sampleV;                        //sample_ holds the raw analog read value
    int sampleI;

    double lastFilteredV,filteredV;          //Filtered_ is the raw analog value minus the DC offset
    double filteredI;
    double offsetV;                          //Low-pass filter output
    double offsetI;                          //Low-pass filter output

    double phaseShiftedV;                             //Holds the calibrated phase shifted voltage.

    double sqV,sumV,sqI,sumI,instP,sumP;              //sq = squared, sum = Sum, inst = instantaneous

    int startV;                                       //Instantaneous voltage at start of sample window.

    boolean lastVCross, checkVCross;                  //Used to measure number of times threshold is crossed.

  int16_t fV[4096];
  int16_t fI[4096];
 
uint32_t queryA0() { 
  int SupplyVoltage=3300;
  //int SupplyVoltage = readVcc();
  const uint16_t timeout = 400;
  unsigned int crossCount = 0;                             //Used to measure number of times threshold is crossed.
  unsigned int numberOfSamples = 0;                        //This is now incremented

  //-------------------------------------------------------------------------------------------------------------------------
  // 1) Waits for the waveform to be close to 'zero' (mid-scale adc) part in sin curve.
  //-------------------------------------------------------------------------------------------------------------------------
  boolean st=false;                                  //an indicator to exit the while loop

  unsigned long start = millis();    //millis()-start makes sure it doesnt get stuck in the loop if there is an error.
/*
  while(st==false)                                   //the while loop...
  {
    startV = analogRead(inPinV);                    //using the voltage waveform
    if ((startV < (ADC_COUNTS*0.55)) && (startV > (ADC_COUNTS*0.45))) st=true;  //check its within range
    if ((millis()-start)>timeout) st = true;
  }
*/
  //-------------------------------------------------------------------------------------------------------------------------
  // 2) Main measurement loop
  //-------------------------------------------------------------------------------------------------------------------------
  start = millis();

  //while ((crossCount < crossings) && ((millis()-start)<timeout))
  while ((millis() - start) < timeout && numberOfSamples < 4096)
  {
    //numberOfSamples++;                       //Count number of times looped.
    //lastFilteredV = filteredV;               //Used for delay/phase compensation

    //-----------------------------------------------------------------------------
    // A) Read in raw voltage and current samples
    //-----------------------------------------------------------------------------
    sampleV = mcp3221_read(MCP_V);                 //Read in raw voltage signal
    sampleI = mcp3221_read(MCP_0);                 //Read in raw current signal
    fV[numberOfSamples] = sampleV;
    fI[numberOfSamples] = sampleI;
    numberOfSamples++;                       //Count number of times looped.
  }
  offsetV = 0;
  offsetI = 0;
  for (uint16_t i = numberOfSamples / 2; i < numberOfSamples; i++) {
    offsetV += fV[i];
    offsetI += fI[i];
  }

  offsetV /= (numberOfSamples / 2);
  offsetI /= (numberOfSamples / 2);
  for (uint16_t i = 0; i < numberOfSamples; i++) {
    fV[i] -= offsetV;
    fI[i] -= offsetI;
  }

  for (uint16_t i = 2; i < numberOfSamples - 2; i++) {
    fV[i] = (fV[i - 2] + fV[i - 1] + fV[i] + fV[i  + 1] + fV[i + 2])/5;
    fI[i] = (fI[i - 2] + fI[i - 1] + fI[i] + fI[i  + 1] + fI[i + 2])/5;
  }
  for (uint16_t i = 2; i < numberOfSamples - 2; i++) {
    fV[i] = (fV[i - 2] + fV[i - 1] + fV[i] + fV[i  + 1] + fV[i + 2])/5;
    fI[i] = (fI[i - 2] + fI[i - 1] + fI[i] + fI[i  + 1] + fI[i + 2])/5;
  }

  for (uint16_t i = numberOfSamples / 2; i < numberOfSamples; i++) {
    sampleV = fV[i];
    sampleI = fI[i];
    lastFilteredV = filteredV;               //Used for delay/phase compensation
    //-----------------------------------------------------------------------------
    // B) Apply digital low pass filters to extract the 2.5 V or 1.65 V dc offset,
    //     then subtract this - signal is now centred on 0 counts.
    //-----------------------------------------------------------------------------
   // offsetV = offsetV + ((sampleV-offsetV)/4096);
    filteredV = sampleV;// - offsetV;
   // offsetI = offsetI + ((sampleI-offsetI)/4096);
    filteredI = sampleI;// - offsetI;

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
    if (sampleV > startV) checkVCross = true;
                     else checkVCross = false;
    if (numberOfSamples==1) lastVCross = checkVCross;

    if (lastVCross != checkVCross) crossCount++;
  }

  //-------------------------------------------------------------------------------------------------------------------------
  // 3) Post loop calculations
  //-------------------------------------------------------------------------------------------------------------------------
  //Calculation of the root of the mean of the voltage and current squared (rms)
  //Calibration coefficients applied.

  double V_RATIO = VCAL *((SupplyVoltage/1000.0) / (ADC_COUNTS));
  Vrmss[ch] = V_RATIO * sqrt(sumV / numberOfSamples);

  double I_RATIO = ICAL *((SupplyVoltage/1000.0) / (ADC_COUNTS));
  Irmss[ch] = I_RATIO * sqrt(sumI / numberOfSamples);

  //Calculation power values
  realPowers[ch] = V_RATIO * I_RATIO * sumP / numberOfSamples;
  apparentPowers[ch] = Vrmss[ch] * Irmss[ch];
  powerFactors[ch] = realPowers[ch] / apparentPowers[ch];

  //Reset accumulators
  sumV = 0;
  sumI = 0;
  sumP = 0;
//--------------------------------------------------------------------------------------
  Serial.println(ch);
  Serial.println(Vrmss[ch]);
  Serial.println(Irmss[ch]);
  Serial.println(realPowers[ch]);
  Serial.println(powerFactors[ch]);
  Serial.println(numberOfSamples);
  ch++;
  if (ch >= MCP_COUNT) ch = 0;
  return A0_DELAY;
}

