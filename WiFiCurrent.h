#pragma once
#include <Filters.h>

float testFrequency = 60;                     // test signal frequency (Hz)
float windowLength = 20.0 / testFrequency;     // how long to average the signal, for statistist
int sensorValue = 0;
float intercept = -0.016945; // to be adjusted based on calibration testing
float slope = 0.0528; // to be adjusted based on calibration testing
					  //float intercept = -0.045; // to be adjusted based on calibration testing
					  //float slope = 0.0495; // to be adjusted based on calibration testing
RunningStatistics inputStats;                 // create statistics to look at the raw test signal

float current_amps; // estimated actual current in amps

unsigned long printPeriod = 300; // in milliseconds
								 // Track time in milliseconds since last reading 
unsigned long previousMillis = 0;

uint32_t initA0() {
  inputStats.setWindowSecs(windowLength);
	return RUN_DELETE;
}

uint32_t queryA0() {
	previousMillis = millis();

	boolean ampLoopflag = true;

	while (ampLoopflag) {

		sensorValue = analogRead(A0);  // read the analog in value:
		inputStats.input(sensorValue);  // log to Stats function

		if ((unsigned long)(millis() - previousMillis) >= printPeriod) {
			previousMillis = millis();   // update time

										 // display current values to the screen
			Serial.print("\n");
			// output sigma or variation values associated with the inputValue itself
			//Serial.print("\tsigma: "); Serial.print(inputStats.sigma());
			// convert signal sigma value to current in amps
			current_amps = intercept + slope * inputStats.sigma();
			//if (current_amps < 0.15) { current_amps = 0.00; }
			//Serial.print("\tamps: "); Serial.print(current_amps);
			//Serial.print("\tWatts: "); Serial.print(current_amps * 230);
			delay(1);
			ampLoopflag = false;
		}
	}
	if (current_amps<0.02) { current_amps = 0; }
	amps = current_amps;
	return A0_DELAY;
}

// Rerurn current value from ACS712 & A0
float current() {
  return amps;
}
