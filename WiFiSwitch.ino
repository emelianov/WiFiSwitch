#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <time.h>
#include <Run.h>
#include <Wire.h>
#include <RTClib.h>

#define SDA D2
#define SCL D3
#define NTP_CHECK_DELAY 300000;
#define NTP_MAX_COUNT 3

RTC_DS3231 rtc;

//#include <FS.h>
//#include <TinyXML.h>

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

void setup(void){
  Serial.begin(74880);
  Wire.begin(SDA,SCL);
  rtcPresent = rtc.begin();
  WiFi.mode(WIFI_OFF);
  //SPIFFS.begin();
  //xml.init((uint8_t *)buffer, sizeof(buffer), &XML_callback);
  //taskAdd(start);
  //taskAddWithSemaphore(initNtp, &event.wifiReady);
}
void loop(void){
  taskExec();
  yield();
}
