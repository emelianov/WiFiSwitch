#pragma once

extern unsigned char ICACHE_RAM_ATTR twi_readFrom(unsigned char address, unsigned char* buf, unsigned int len, unsigned char sendStop);

bool mcp3221_init(uint32_t freq = 400000, uint8_t sda = 0, uit8_t scl = 0) {
//  if (sda != 0 || sdl != 0) twi_init(sda, sdl);
//  twi_setClock(freq);
//  twi_setClockStretchLimit(230); // default value is 230 uS
}

uint16_t ICACHE_RAM_ATTR mcp3221_getData() {
  uint16_t rawData = 0;

  if(twi_readFrom(address, &rawData, 2, true) != 0)
    return 0;
  return rawData;
}