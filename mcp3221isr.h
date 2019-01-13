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
//#include <Arduino.h>

//extern void twi_iram_setClock(unsigned int freq);
//extern void twi_iram_setClockStretchLimit(uint32_t limit);
//extern void twi_iram_init(unsigned char sda, unsigned char scl);
//static void ICACHE_RAM_ATTR twi_iram_delay(unsigned char v);
//static bool ICACHE_RAM_ATTR twi_iram_write_start(void);
//static bool ICACHE_RAM_ATTR twi_iram_write_stop(void);
//static bool ICACHE_RAM_ATTR twi_iram_write_bit(bool bit);
//static bool ICACHE_RAM_ATTR twi_iram_read_bit(void);
//static bool ICACHE_RAM_ATTR twi_iram_write_byte(unsigned char byte);
//static unsigned char ICACHE_RAM_ATTR twi_iram_read_byte(bool nack);
//unsigned char ICACHE_RAM_ATTR twi_iram_writeTo(unsigned char address, unsigned char * buf, unsigned int len, unsigned char sendStop);
//extern unsigned char ICACHE_RAM_ATTR twi_iram_readFrom(unsigned char address, unsigned char* buf, unsigned int len, unsigned char sendStop);

#define ADC_BITS    12

#ifndef __bswap_16
 #define __bswap_16(num) ((uint16_t)num>>8) | ((uint16_t)num<<8)
#endif


/*
  si2c.c - Software I2C library for esp8266

  Copyright (c) 2015 Hristo Gochkov. All rights reserved.
  This file is part of the esp8266 core for Arduino environment.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

  No changes from original except ICACHE_RAM_ATTR added to funcs used for reading I2C.
  This is required to read I2C from interrupt routine.
*/
//#include "twi.h"
//#include "pins_arduino.h"
//#include "wiring_private.h"
//#include "core_esp8266_si2c.h"

//#include "twi.h"
//#include "pins_arduino.h"
//#include "wiring_private.h"

unsigned int twi_iram_preferred_si2c_clock = 100000;
unsigned char twi_iram_dcount = 18;
static unsigned char twi_sda, twi_scl;
static uint32_t twi_iram_clockStretchLimit;

#define SDA_LOW()   (GPES = (1 << twi_sda)) //Enable SDA (becomes output and since GPO is 0 for the pin, it will pull the line low)
#define SDA_HIGH()  (GPEC = (1 << twi_sda)) //Disable SDA (becomes input and since it has pullup it will go high)
#define SDA_READ()  ((GPI & (1 << twi_sda)) != 0)
#define SCL_LOW()   (GPES = (1 << twi_scl))
#define SCL_HIGH()  (GPEC = (1 << twi_scl))
#define SCL_READ()  ((GPI & (1 << twi_scl)) != 0)

#ifndef FCPU80
#define FCPU80 80000000L
#endif

#if F_CPU == FCPU80
#define TWI_CLOCK_STRETCH_MULTIPLIER 3
#else
#define TWI_CLOCK_STRETCH_MULTIPLIER 6
#endif

void twi_iram_setClock(unsigned int freq){
  twi_iram_preferred_si2c_clock = freq;
#if F_CPU == FCPU80
  if(freq <= 50000) twi_iram_dcount = 38;//about 50KHz
  else if(freq <= 100000) twi_iram_dcount = 19;//about 100KHz
  else if(freq <= 200000) twi_iram_dcount = 8;//about 200KHz
  else if(freq <= 300000) twi_iram_dcount = 3;//about 300KHz
  else if(freq <= 400000) twi_iram_dcount = 1;//about 400KHz
  else twi_iram_dcount = 1;//about 400KHz
#else
  if(freq <= 50000) twi_iram_dcount = 64;//about 50KHz
  else if(freq <= 100000) twi_iram_dcount = 32;//about 100KHz
  else if(freq <= 200000) twi_iram_dcount = 14;//about 200KHz
  else if(freq <= 300000) twi_iram_dcount = 8;//about 300KHz
  else if(freq <= 400000) twi_iram_dcount = 5;//about 400KHz
  else if(freq <= 500000) twi_iram_dcount = 3;//about 500KHz
  else if(freq <= 600000) twi_iram_dcount = 2;//about 600KHz
  else twi_iram_dcount = 1;//about 700KHz
#endif
}

void twi_iram_setClockStretchLimit(uint32_t limit){
  twi_iram_clockStretchLimit = limit * TWI_CLOCK_STRETCH_MULTIPLIER;
}

void twi_iram_init(unsigned char sda, unsigned char scl){
  twi_sda = sda;
  twi_scl = scl;
  pinMode(twi_sda, INPUT_PULLUP);
  pinMode(twi_scl, INPUT_PULLUP);
  twi_iram_setClock(twi_iram_preferred_si2c_clock);
  twi_iram_setClockStretchLimit(230); // default value is 230 uS
}

static void ICACHE_RAM_ATTR twi_iram_delay(unsigned char v){
  unsigned int i;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"
  unsigned int reg;
  for(i=0;i<v;i++) 
    reg = GPI;
#pragma GCC diagnostic pop
}

static bool ICACHE_RAM_ATTR twi_iram_write_start(void) {
  SCL_HIGH();
  SDA_HIGH();
  if (SDA_READ() == 0) 
    return false;
  twi_iram_delay(twi_iram_dcount);
  SDA_LOW();
  twi_iram_delay(twi_iram_dcount);
  return true;
}

static bool ICACHE_RAM_ATTR twi_iram_write_stop(void){
  uint32_t i = 0;
  SCL_LOW();
  SDA_LOW();
  twi_iram_delay(twi_iram_dcount);
  SCL_HIGH();
  while (SCL_READ() == 0 && (i++) < twi_iram_clockStretchLimit); // Clock stretching
  twi_iram_delay(twi_iram_dcount);
  SDA_HIGH();
  twi_iram_delay(twi_iram_dcount);

  return true;
}

static bool ICACHE_RAM_ATTR twi_iram_write_bit(bool bit) {
  uint32_t i = 0;
  SCL_LOW();
  if (bit) 
    SDA_HIGH();
  else 
    SDA_LOW();
  twi_iram_delay(twi_iram_dcount+1);
  SCL_HIGH();
  while (SCL_READ() == 0 && (i++) < twi_iram_clockStretchLimit);// Clock stretching
  twi_iram_delay(twi_iram_dcount);
  return true;
}

static bool ICACHE_RAM_ATTR twi_iram_read_bit(void) {
  uint32_t i = 0;
  SCL_LOW();
  SDA_HIGH();
  twi_iram_delay(twi_iram_dcount+2);
  SCL_HIGH();
  while (SCL_READ() == 0 && (i++) < twi_iram_clockStretchLimit);// Clock stretching
  bool bit = SDA_READ();
  twi_iram_delay(twi_iram_dcount);
  return bit;
}

static bool ICACHE_RAM_ATTR twi_iram_write_byte(unsigned char byte) {
  unsigned char bit;
  for (bit = 0; bit < 8; bit++) {
    twi_iram_write_bit(byte & 0x80);
    byte <<= 1;
  }
  return !twi_iram_read_bit();//NACK/ACK
}

static unsigned char ICACHE_RAM_ATTR twi_iram_read_byte(bool nack) {
  unsigned char byte = 0;
  unsigned char bit;
  for (bit = 0; bit < 8; bit++) 
    byte = (byte << 1) | twi_iram_read_bit();
  twi_iram_write_bit(nack);
  return byte;
}

unsigned char ICACHE_RAM_ATTR twi_iram_writeTo(unsigned char address, unsigned char * buf, unsigned int len, unsigned char sendStop){
  unsigned int i;
  if(!twi_iram_write_start()) 
    return 4;//line busy
  if(!twi_iram_write_byte(((address << 1) | 0) & 0xFF)) {
    if (sendStop) 
      twi_iram_write_stop();
    return 2; //received NACK on transmit of address
  }
  for(i=0; i<len; i++) {
    if(!twi_iram_write_byte(buf[i])) {
      if (sendStop) 
        twi_iram_write_stop();
      return 3;//received NACK on transmit of data
    }
  }
  if(sendStop) 
    twi_iram_write_stop();
  i = 0;
  while(SDA_READ() == 0 && (i++) < 10){
    SCL_LOW();
    twi_iram_delay(twi_iram_dcount);
    SCL_HIGH();
    twi_iram_delay(twi_iram_dcount);
  }
  return 0;
}

unsigned char ICACHE_RAM_ATTR twi_iram_readFrom(unsigned char address, unsigned char* buf, unsigned int len, unsigned char sendStop){
  unsigned int i;
  if(!twi_iram_write_start()) 
    return 4;//line busy
  if(!twi_iram_write_byte(((address << 1) | 1) & 0xFF)) {
    if (sendStop) 
      twi_iram_write_stop();
    return 2;//received NACK on transmit of address
  }
  for(i=0; i<(len-1); i++) 
    buf[i] = twi_iram_read_byte(false);
  buf[len-1] = twi_iram_read_byte(true);
  if(sendStop) 
    twi_iram_write_stop();
  i = 0;
  while(SDA_READ() == 0 && (i++) < 10){
    SCL_LOW();
    twi_iram_delay(twi_iram_dcount);
    SCL_HIGH();
    twi_iram_delay(twi_iram_dcount);
  }
  return 0;
}

/*
void twi_stop(void){
  pinMode(twi_sda, INPUT);
  pinMode(twi_scl, INPUT);
}

uint8_t twi_status() {           
    if (SCL_READ()==0)     
        return I2C_SCL_HELD_LOW;             //SCL held low by another device, no procedure available to recover
    int clockCount = 20;                   

    while (SDA_READ()==0 && clockCount>0) {                     //if SDA low, read the bits slaves have to sent to a max
        --clockCount;
        twi_iram_read_bit();                    
        if (SCL_READ()==0) 
            return I2C_SCL_HELD_LOW_AFTER_READ;  //I2C bus error. SCL held low beyond slave clock stretch time
    }

    if (SDA_READ()==0)     
        return I2C_SDA_HELD_LOW;             //I2C bus error. SDA line held low by slave/another_master after n bits.

    if(!twi_iram_write_start()) 
        return I2C_SDA_HELD_LOW_AFTER_INIT;  //line busy. SDA again held low by another device. 2nd master?

    return I2C_OK;                       //all ok

}

*/



bool mcp3221_init(uint32_t freq, uint8_t sda, uint8_t scl) {
  if (sda != 0 || scl != 0) twi_iram_init(sda, scl);
  twi_iram_setClock(freq);
  twi_iram_setClockStretchLimit(230); // default value is 230 uS
}

uint16_t ICACHE_RAM_ATTR mcp3221_read(uint8_t address) {
  uint16_t rawData = 0;

  if(twi_iram_readFrom(address, (uint8_t*)&rawData, 2, true) != 0)
    return 1;
  return __bswap_16(rawData);
}
