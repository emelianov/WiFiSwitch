#pragma once
#include <ets_sys.h>
#include <Arduino.h>
#include "mcp3221isr.h"

#define MES_COUNT 1000

volatile bool first = true;
volatile uint32_t cTm = 0;

volatile int maxValue = 0;          // store max value here
volatile int minValue = 65535;          // store min value here

volatile uint16_t curMax = 0;
volatile uint16_t curMin = 65535;
volatile uint16_t curCounter = 0;
volatile uint16_t readValue;
volatile bool adcBusy = false;

void ICACHE_RAM_ATTR timer_isr(){
  if (adcBusy) return;
  uint32_t t = micros();
  adcBusy = true;
  // Level 1 is used Wi-Fi stack.
  // Level 2 Debug
  // Level 3 NMI (timers?)
  xt_rsil(1);
  //readValue = mcp3221_read(0x49);
  readValue = mcp3221_read(0x4A);
  //readValue = mcp3221_read(0x49);
  if (readValue > curMax)
    curMax = readValue;
  if (readValue < curMin) 
    curMin = readValue;
  curCounter++;
  if (curCounter >= MES_COUNT) {
    minValue = curMin;
    maxValue = curMax;
    curMax = 0;
    curMin = 65535;
    curCounter = 0;
    if (first) cTm = micros() - t;
    first = false;
  }
  adcBusy = false;
}
