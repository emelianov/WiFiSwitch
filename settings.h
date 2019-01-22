#pragma once
// Settings mostly for AC power meter

// Switch to debug NodeNCU 1.0 mode
// If not defined -- production WeMos mini
//#define WFS_DEBUG


#define MCP_COUNT 3

// Voltage/Current aproximation window size
#define APPROX_WINDOW 5

#ifdef WFS_DEBUG
//Debug settings
 #define WDEBUG(format, ...) Serial.printf_P(PSTR(format), ##__VA_ARGS__);

 #define SDA 4
 #define SCL 5

 #define MCP_V 0x49
 #define MCP_0 0x4A
 #define MCP_1 0x4A
 #define MCP_3 0x4A

 #define DEF_VCAL     1.0
 #define DEF_ICAL     15.15
 #define DEF_PHASECAL  2.75
 #define DEF_SUPPLY   3300

#else
//Production settings
 #define WDEBUG(format, ...)
// I2S bus pins
 #define SDA D2
 #define SCL D3

// MCP3221 addresses
 #define MCP_V 0x4E
 #define MCP_1 0x4F
 #define MCP_3 0x4C
 #define MCP_0 0x4D

// Voltage callibration
 #define DEF_VCAL     1.0
// Current callibration
 #define DEF_ICAL     15.15
// Voltage shift callibration
 #define DEF_PHASECAL  2.75
// ADC power voltage
 #define DEF_SUPPLY   3300
#endif
