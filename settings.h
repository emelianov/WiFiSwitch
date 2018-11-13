#pragma once
// Settings mostly for AC power meter

// Switch to debug NodeNCU 1.0 mode
// If not defined -- production WeMos mini
//#define WFS_DEBUG

// I2S bus pins
#ifdef WFS_DEBUG
 #define SDA 4
 #define SCL 5
#else
 #define SDA D2
 #define SCL D3
#endif

#define MAX_SAMPLES 1200

// MCP3221 addresses
#define MCP_COUNT 3
#ifdef WFS_DEBUG
 #define MCP_V 0x49
 #define MCP_0 0x4A
 #define MCP_1 0x4A
 #define MCP_3 0x4A
//uint8_t sampleI[MCP_COUNT] = {MCP_0, MCP_1, MCP_3};
 #define DEF_VCAL     15000.0
 #define DEF_ICAL     50.0
 #define DEF_PHASECAL  2.75
 #define DEF_SUPPLY   3300
#else
// Voltage
 #define MCP_V 0x4D
//J4
 #define MCP_0 0x4C
//J5
 #define MCP_1 0x4E
//J2
 #define MCP_3 0x4F
// Voltage vallibration
 #define DEF_VCAL     15000.0
// Current callibration
 #define DEF_ICAL     200.0
// Voltage shift callibration
 #define DEF_PHASECAL  2.75
// ADC power voltage
 #define DEF_SUPPLY   3300
#endif

// Voltage/Current aproximation window size
#define APPROX_WINDOW 5
