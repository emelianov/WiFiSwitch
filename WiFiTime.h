#pragma once
#include <Wire.h>
#include <time.h>
#include <RTClib.h>

#define SDA D2
#define SCL D3
#define NTP_CHECK_DELAY 30000;

RTC_DS3231 rtc;
uint16_t timeZone;
//Update time from NTP server
uint32_t initNTP() {
  timeZone = atoi(tz.c_str()) * 3600;
  if (time(NULL) == 0) {
    configTime(timeZone, 0, ntp1.c_str(), ntp2.c_str(), ntp3.c_str());
    return NTP_CHECK_DELAY;
  }
  status.ntpSync = true;
  if (status.rtcPresent) {
    rtc.adjust(DateTime(time(NULL)-timeZone));
    status.rtcValid = !rtc.lostPower();
  }
  return RUN_DELETE;
}

time_t getTime() {
  time_t t = 0;
  if (status.rtcValid) {
    DateTime now = rtc.now();
    t = now.unixtime()+timeZone;
  } else {
    if (status.ntpSync) {
      t = time(NULL);
    }
  }
  return t % 86400;
}

uint32_t initRTC() {
  Wire.begin(SDA,SCL);
  status.rtcPresent = rtc.begin();
  status.rtcValid = !rtc.lostPower();
  return RUN_DELETE;
}