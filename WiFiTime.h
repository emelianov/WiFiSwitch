#include <Wire.h>
#include <time.h>
#include <RTClib.h>

#define SDA D2
#define SCL D3
#define NTP_CHECK_DELAY 300000;
#define NTP_MAX_COUNT 3

RTC_DS3231 rtc;

//Update time from NTP server
uint32_t initNtp() {
  if (time(NULL) == 0) {
    configTime(atoi(tz.c_str()) * 3600, 0, ntp1.c_str(), ntp2.c_str(), ntp3.c_str());
    return NTP_CHECK_DELAY;
  }
  status.ntpSync = true;
  if (status.rtcPresent) {
    rtc.adjust(DateTime(time(NULL)));
    status.rtcValid = rtc.lostPower();
  }
  return RUN_DELETE;
}

time_t getTime() {
  if (status.rtcValid) {
  } else {
    if (status.ntpSync) {
      return time(NULL);
    }
  }
  return 0;
}

uint32_t initRTC() {
  Wire.begin(SDA,SCL);
  status.rtcPresent = rtc.begin();
  status.rtcValid = rtc.lostPower();
  return RUN_DELETE;
}
