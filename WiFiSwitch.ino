#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
#include <Run.h>

//#include <FS.h>
//#include <TinyXML.h>

struct events {
 uint8_t wifiConnected		= 0;
 uint8_t resetLongPress 	= 0;
 uint8_t resetShortPress	= 0;
 uint8_t turnOffAllSockets	= 0;
}
struct status {
 bool wifiConnected		= false;
 bool ntpSync			= false;
 bool rtcPresent		= false;
 bool rtcValid			= false;
}
void setup(){
  Serial.begin(74880);
  WiFi.mode(WIFI_OFF);
  //SPIFFS.begin();
  //xml.init((uint8_t *)buffer, sizeof(buffer), &XML_callback);
  //taskAdd(start);
  //taskAddWithSemaphore(initNtp, &event.wifiReady);

  WiFiManager wifiManager;
  //wifiManager.resetSettings();
  wifiManager.setTimeout(180);
  while(!wifiManager.autoConnect("AutoConnectAP")) {
    Serial.println("failed to connect and hit timeout");
    delay(5000);
  } 

  //if you get here you have connected to the WiFi
  Serial.println("connected...yeey :)");

}
void loop(void){
  taskExec();
  yield();
}
