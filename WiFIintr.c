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

uint16_t intrAction = READ_V;
volatile uint16_t samples = 0;

uint16_t ctr = 0;
// Buffer for samples store used signed int as ADC is 12-bit and faser zero normalization will be performed in-place.
int16_t sV[MAX_SAMPLES] = {0};
int16_t sI[MAX_SAMPLES] = {0};
int16_t sW[MAX_SAMPLES] = {0};

// Be carefull changing interrupt code
// mcp3221_read() takes ~95microSeconds
// Interrupt must not take above 100-150microSeconds
// Sum of interrupts dutation should not exceed 200-250milliSeconds per second.
// Should not use calls of eny external functions (except ICACHE_RAM_ATTR marked) and float point calculations
// Breaking these limitations leads to controller reset by watchdog ater some time.
void ICACHE_RAM_ATTR timer_isr(){
  if (mcpDataReady > 0) return;
  if (adcBusy) {
    ctr = 0;
    return; // Need add reset of data collection 
  }
  adcBusy = true;
  // Level 1 is used Wi-Fi stack.
  // Level 2 Debug
  // Level 3 NMI (timers?)
  xt_rsil(1);

  if (ctr < MAX_SAMPLES) {
    if (intrAction == READ_V) {
      intrAction = READ_I;
      sV[ctr] = mcp3221_read(MCP_V);
    } else {
      intrAction = READ_V;
      sI[ctr] = mcp3221_read(mcpA[mcp]);
      ctr++;
    }
  } else {
      intrAction = READ_V;
      mcpCalc = mcp;
      mcp++;
      if (mcp >= MCP_COUNT) mcp = 0;
      ctr = 0;
      mcpDataReady++;
  }

 cleanup:
  adcBusy = false;
}
