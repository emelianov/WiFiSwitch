/*
 * 
 * MCP3221 read routine
 * 
 * Raw reading time about 0.1mS
 * 
 */
#pragma once
#include <Wire.h>
#include <twi.h>

#define ADC_BITS    12

#ifndef __bswap_16
 #define __bswap_16(num) ((uint16_t)num>>8) | ((uint16_t)num<<8)
#endif

bool mcp3221_init(uint32_t freq, uint8_t sda, uint8_t scl) {
  Wire.begin(sda, scl);
  Wire.beginTransmission(MCP_V);
  Wire.endTransmission();
  Wire.beginTransmission(MCP_0);
  Wire.endTransmission();
  if (sda != 0 || scl != 0) twi_init(sda, scl);
  twi_setClock(freq);
  twi_setClockStretchLimit(230); // default value is 230 uS
}

uint16_t mcp3221_read(uint8_t address) {
  uint16_t rawData = 0;

  if(twi_readFrom(address, (uint8_t*)&rawData, 2, true) != 0)
    return 1;
  return __bswap_16(rawData);
}
