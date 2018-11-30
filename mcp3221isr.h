/*
 * 
 * MCP3221 read routine for usage inside interrupt
 * Requirements:
 * - core_esp8266_si2c.c patched with ICHACHE_RAM_ATTR added to all function required for I2C reading.
 * - 400kHz bus speed
 * - interrupt sampling rate should not exceed ~3000Hz
 * 
 * Raw reading time about 0.1mS
 * 
 */
#pragma once
#include <Arduino.h>

#define ADC_BITS    12

#ifndef __bswap_16
 #define __bswap_16(num) ((uint16_t)num>>8) | ((uint16_t)num<<8)
#endif

extern unsigned char ICACHE_RAM_ATTR twi_readFrom(unsigned char address, unsigned char* buf, unsigned int len, unsigned char sendStop);

bool mcp3221_init(uint32_t freq, uint8_t sda, uint8_t scl) {
  if (sda != 0 || scl != 0) twi_init(sda, scl);
  twi_setClock(freq);
  twi_setClockStretchLimit(230); // default value is 230 uS
}

uint16_t ICACHE_RAM_ATTR mcp3221_read(uint8_t address) {
  uint16_t rawData = 0;

  if(twi_readFrom(address, (uint8_t*)&rawData, 2, true) != 0)
    return 1;
  return __bswap_16(rawData);
}
