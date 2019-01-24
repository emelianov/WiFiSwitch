#pragma once
#include <Wire.h>
#include <time.h>
#include <RTClib.h>
/*
int _EXFUN(settimeofday, (const struct timeval *, const struct timezone *));
struct timezone {
  int tz_minuteswest;
  int tz_dsttime;
};
struct timeval {
  time_t      tv_sec;
  suseconds_t tv_usec;
};
*/

#define NTP_CHECK_DELAY 300000L
// 01/01/2018
#define DEF_TIME 1514764800

RTC_DS3231 rtc;
int32_t timeZone = 0;
uint8_t ntpId = 0;
//Update time from NTP server
uint32_t initNTP() {
  WDEBUG("TimeZone: %s, TimeOffset: %d\n", tz.c_str(), timeZone);
  timeZone = atoi(tz.c_str()) * 3600;
 /*
  if (time(NULL) < DEF_TIME) {
    if (ntpId == 0) {
      configTime(timeZone, 0, ntp1.c_str());
      ntpId++;
    } else if (ntpId == 1) {
      configTime(timeZone, 0, ntp2.c_str());
      ntpId++;
    } else {
      configTime(timeZone, 0, ntp3.c_str());
      ntpId = 0;
    }
  */
  configTime(timeZone, 0, ntp1.c_str(), ntp2.c_str(), ntp3.c_str());
  //  return NTP_CHECK_DELAY;
  //}
  if (time(NULL) > DEF_TIME) {
    status.ntpSync = true;
    if (status.rtcPresent) {
      rtc.adjust(DateTime(time(NULL)));
      status.rtcValid = !rtc.lostPower();
    }
  }
  return NTP_CHECK_DELAY;
}

time_t getTime() {
  time_t t = 0;
/*  if (!rtc.lostPower()) {
    DateTime now = rtc.now();
    t = now.unixtime()+timeZone;
  } else { */
    //if (status.ntpSync) {
      t = time(NULL);
    //}
//  }
  return t % 86400;
}
  
uint32_t initRTC() {
  Wire.begin(SDA,SCL);
  rtc.begin();
  Wire.beginTransmission(DS3231_ADDRESS);             // Check if RTC is 
  status.rtcPresent = (Wire.endTransmission() == 0);  // present
  status.rtcValid = !rtc.lostPower();
  
  if (status.rtcValid) {
    //int _EXFUN(settimeofday, (const struct timeval *, const struct timezone *));
    timezone tzs;
    tzs.tz_minuteswest = timeZone / 60;
    tzs.tz_dsttime = 0;
    timeval tvs;
    DateTime now = rtc.now();
    tvs.tv_sec = now.unixtime();
    tvs.tv_usec = 0;
    settimeofday(&tvs, &tzs);
  }

  return RUN_DELETE;
}