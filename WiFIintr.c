#include <Arduino.h>
#include <twi.h>

#define MES_COUNT 1000

volatile bool first = true;
volatile uint32_t cTm = 0;

volatile int maxValue = 0;          // store max value here
volatile int minValue = 1024;          // store min value here

volatile uint16_t curMax = 0;
volatile uint16_t curMin = 1024;
volatile uint16_t curCounter = 0;
volatile uint16_t readValue;
volatile bool adcBusy = false;

//extern unsigned char twi_dcount;
unsigned char twi_sda = 4;
unsigned char twi_scl = 5;
//extern uint32_t twi_clockStretchLimit;

#define SDA_LOW()   (GPES = (1 << twi_sda)) //Enable SDA (becomes output and since GPO is 0 for the pin, it will pull the line low)
#define SDA_HIGH()  (GPEC = (1 << twi_sda)) //Disable SDA (becomes input and since it has pullup it will go high)
#define SDA_READ()  ((GPI & (1 << twi_sda)) != 0)
#define SCL_LOW()   (GPES = (1 << twi_scl))
#define SCL_HIGH()  (GPEC = (1 << twi_scl))
#define SCL_READ()  ((GPI & (1 << twi_scl)) != 0)

void ICACHE_RAM_ATTR timer_isr(){
  if (adcBusy) return;
  uint32_t t = micros();
  adcBusy = true;
  //readValue = analogRead(A0);
  //sei();
  //Wire.requestFrom(0x68, 0x0F);
  //if (Wire.available() > 0) uint16_t rawData = (Wire.read() << 8) | (Wire.read());
  //cli();
  unsigned char byte = 10;
  unsigned char bit;
  for (bit = 0; bit < 8; bit++) {
    uint8_t bit = byte & 0x80;
    uint32_t i = 0;
    SCL_LOW();
    if (bit) 
      SDA_HIGH();
    else 
      SDA_LOW();
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"
  unsigned int reg;
  for(uint16_t i=0;i<50 + 1;i++) 
    reg = GPI;
#pragma GCC diagnostic pop
    SCL_HIGH();
//    while (SCL_READ() == 0 && (i++) < twi_clockStretchLimit);// Clock stretching
//    twi_delay(twi_dcount);
    
    byte <<= 1;
  }
//  return !twi_read_bit();//NACK/ACK

  if (readValue > curMax)
    curMax = readValue;
  if (readValue < curMin) 
    curMin = readValue;
  curCounter++;
  if (curCounter >= MES_COUNT) {
    minValue = curMin;
    maxValue = curMax;
    curMax = 0;
    curMin = 1024;
    curCounter = 0;
    if (first) cTm = micros() - t;
    first = false;
  }
  adcBusy = false;
}
