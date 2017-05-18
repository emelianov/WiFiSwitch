#include <time.h>
#include <RTClib.h>

#define SDA D2
#define SCL D3
#define NTP_CHECK_DELAY 300000;
#define NTP_MAX_COUNT 3

RTC_DS3231 rtc;

//NTP settings
String timeServer[NTP_MAX_COUNT];         // NTP servers
int8_t timeZone = 0;
bool rtcPresent = false;

//Update time from NTP server
uint32_t initNtp() {
  if (time(NULL) == 0) {
    configTime(timeZone * 3600, 0, timeServer[0].c_str(), timeServer[1].c_str(), timeServer[2].c_str());
    return NTP_CHECK_DELAY;
  }
  if (rtcPresent) {
    rtc.adjust(DataTime(time(NULL)));
  }
  return RUN_DELETE;
}

function getTime() {
  
}

void initTime() {
  Wire.begin(SDA,SCL);
  rtcPresent = rtc.begin();
  taskAdd(initNtp);
}