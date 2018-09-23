#pragma once
#include "EmonLib.h" 
EnergyMonitor emon; 

uint32_t initA0() {
  emon.current(A0, 111.1);
  return RUN_DELETE;
}

uint32_t queryA0() {
  amps = emon.calcIrms(870); //1480 = 170mS, 870 = 100mS
  return A0_DELAY;
}

// Rerurn current value from ACS712 & A0
float current() {
  return amps;
}
