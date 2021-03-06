#pragma once
// Settings mostly for AC power meter

#define MCP_COUNT 3

// Voltage/Current aproximation window size
#define APPROX_WINDOW 5

// Switch to debug NodeNCU 1.0 mode
// If not defined -- production WeMos mini
#ifdef ARDUINO_ESP8266_NODEMCU
 #define WFS_DEBUG
 #define WFS_HISTORY
//Debug settings
 #define WDEBUG(format, ...) Serial.printf_P((format), ##__VA_ARGS__);

 #define SDA 4
 #define SCL 5

 #define MCP_V 0x49
 #define MCP_0 0x4A
 #define MCP_1 0x4A
 #define MCP_3 0x4A
 #define RAW_NOISE_FLOOR 2
 #define I_NOISE_FLOOR 0.01
 #define DEF_VCAL     90*60
 #define DEF_ICAL     900
 #define DEF_PHASECAL  1
 #define DEF_SUPPLY   3300
 #define VOLTAGE 220
 #define HZ 50

#else
//Production settings
 #define WDEBUG(format, ...)
// I2S bus pins
 #define SDA D2
 #define SCL D3

// MCP3221 addresses
 #define MCP_V 0x4E
 #define MCP_0 0x4C
 #define MCP_1 0x4D
 #define MCP_3 0x4F

// Noise truncation level
// Strip level just after ADC
 #define RAW_NOISE_FLOOR 2
// Strip level acter calculations
 #define I_NOISE_FLOOR 0.01
// Voltage callibration
 #define DEF_VCAL     144.5
// Current callibration
 #define DEF_ICAL    23.0
// Voltage shift callibration
 #define DEF_PHASECAL  1
// ADC power voltage
 #define DEF_SUPPLY   3000
// Wall AC voltage params
 #define VOLTAGE 110
 #define HZ 60
#endif
