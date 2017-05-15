#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <time.h>
#include <Run.h>
//#include <FS.h>
//#include <TinyXML.h>

//NTP settings
#define NTP_CHECK_DELAY 300000;
#define NTP_MAX_COUNT 3
String timeServer[NTP_MAX_COUNT];         // NTP servers
int8_t timeZone = 0;

//Update time from NTP server
uint32_t initNtp() {
  if (time(NULL) == 0) {
    configTime(timeZone * 3600, 0, timeServer[0].c_str(), timeServer[1].c_str(), timeServer[2].c_str());
    return NTP_CHECK_DELAY;
  }
  return RUN_DELETE;
}

void setup(void){
  wdt_enable(0);
  Serial.begin(74880);
  WiFi.mode(WIFI_OFF);
  //SPIFFS.begin();
  //xml.init((uint8_t *)buffer, sizeof(buffer), &XML_callback);
  //taskAdd(start);
  //taskAddWithSemaphore(initNtp, &event.wifiReady);
}
void loop(void){
  taskExec();
  //wdt_reset();
  yield();
}
