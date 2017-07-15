#pragma once
// Using RMS
// Should set ADC_ZERO according to specific ACS chip

// relative digital zero of the arudino input from ACS712 (could make this a variable and auto-adjust it)
#define ADC_ZERO 500
#define MAX_AMPS 30

uint32_t queryA0() {
  const unsigned long sampleTime = 100000UL;                           // sample over 100ms, it is an exact number of cycles for both 50Hz and 60Hz mains
  const unsigned long numSamples = 250UL;                               // choose the number of samples to divide sampleTime exactly, but low enough for the ADC to keep up
  const unsigned long sampleInterval = sampleTime / numSamples;  // the sampling interval, must be longer than then ADC conversion time
  uint32_t currentAcc = 0;
  uint16_t count = 0;
  uint32_t prevMicros = micros() - sampleInterval;
  while (count < numSamples){
    if (micros() - prevMicros >= sampleInterval){
      int16_t adc_raw = analogRead(A0) - ADC_ZERO;
      //Serial.println(adc_raw);
      currentAcc += (unsigned long)(adc_raw * adc_raw);
      ++count;
      prevMicros += sampleInterval;
    }
  }
  float rms = sqrt((float)currentAcc / (float)numSamples) * (MAX_AMPS / 1024.0);
  rms = rms - 0.00;
  //if (rms<0.20){rms = 0;}
  amps = rms;
  return A0_DELAY;
}

uint32_t initA0() {
  return RUN_DELETE;
}

// Rerurn current value from ACS712 & A0
float current() {
  return amps;
}

