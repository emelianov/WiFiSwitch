#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <WiFiManager.h>
#include <Run.h>

#define RESET_PIN D8

struct events {
 uint16_t wifiConnected		= 0;
 uint16_t resetLongPress 	= 0;
 uint16_t resetShortPress	= 0;
 uint16_t turnOffAllSockets= 0;
 uint16_t keyPressed      = 0;
 uint16_t keyReleased     = 0;
};
struct statuses {
 bool wifiConnected	= false;
 bool ntpSync			  = false;
 bool rtcPresent		= false;
 bool rtcValid			= false;
 bool keyPressed    = false;
};
events event;
statuses status;

bool   dhcp = true;
String ip   = "192.168.20.99";
String mask = "255.255.255.0";
String gw   = "192.168.20.2";
String dns  = "192.168.20.2";
String ntp1 = "192.168.30.30";
String ntp2 = "192.168.30.4";
String ntp3 = "pool1.ntp.org";
String tz   = "0";

#include "WiFiTime.h"
#include "WiFiControl.h"

#define WIFI_SETUP_AP "AutoConnectAP"
#define WIFI_CHECK_DELAY 1000
#define WIFI_CHECK_COUNT 5
uint32_t readConfig() {
}
uint32_t wifiStart() {
  WiFi.mode(WIFI_STA);
  if (!dhcp) {
   //WiFi.config();
  }
  WiFi.begin();
  Serial.print("Connecting to ");
  Serial.println(WiFi.SSID());
  taskAddWithDelay(wifiWait, WIFI_CHECK_DELAY);
  return RUN_DELETE;
}
uint8_t waitWF = 1;
uint32_t wifiWait() {
  if(WiFi.status() != WL_CONNECTED) {
    if (waitWF <= WIFI_CHECK_COUNT) {
      Serial.println("Waiting Wi-Fi");
      if (waitWF > 0) {
        waitWF++;
      }
      return WIFI_CHECK_DELAY;
    } else {
      WiFi.mode(WIFI_OFF);
      taskAddWithDelay(wifiStart, WIFI_CHECK_DELAY);
    }
  } else {
    Serial.println(WiFi.localIP());
    event.wifiConnected++;
  }
  return RUN_DELETE;
}
void cbSaveParams(WiFiManager *wfm) {
  if (!dhcp) {
   IPAddress _ip, _gw, _mask;
   _ip.fromString(ip);
   _gw.fromString(gw);
   _mask.fromString(mask);
   wfm->setSTAStaticIPConfig(_ip, _gw, _mask);
  }
}
void cbConnected(WiFiManager *wfm) {
}
uint32_t wifiManager() {
  WiFiManagerParameter pNet("IP/Mask/Gw/DNS");
  WiFiManagerParameter pIp("ip", "IP", ip.c_str(), 16);
  WiFiManagerParameter pMask("mask", "Network mask", mask.c_str(), 16);
  WiFiManagerParameter pGw("gw", "Default gateway", gw.c_str(), 16);
  WiFiManagerParameter pDns("dns", "DNS", dns.c_str(), 16);
  WiFiManagerParameter pNtp("NTP Servers");
  WiFiManagerParameter pNtp1("ntp1", "NTP server", ntp1.c_str(), 40);
  WiFiManagerParameter pNtp2("ntp2", "NTP server", ntp2.c_str(), 40);
  WiFiManagerParameter pNtp3("ntp3", "NTP server", ntp3.c_str(), 40);
  WiFiManagerParameter pTz("tz", "Time Zone", tz.c_str(), 4);
  
  WiFiManager wifiManager;
  //wifiManager.resetSettings();
  wifiManager.setTimeout(180);
  wifiManager.addParameter(&pNet);
  wifiManager.addParameter(&pIp);
  wifiManager.addParameter(&pMask);
  wifiManager.addParameter(&pGw);
  wifiManager.addParameter(&pDns);
  wifiManager.addParameter(&pNtp);
  wifiManager.addParameter(&pNtp1);
  wifiManager.addParameter(&pNtp2);
  wifiManager.addParameter(&pNtp3);
  wifiManager.addParameter(&pTz);
  wifiManager.setConnectTimeout(WIFI_CHECK_DELAY * WIFI_CHECK_COUNT);
  if (!dhcp) {
   IPAddress _ip, _gw, _mask;
   _ip.fromString(ip);
   _gw.fromString(gw);
   _mask.fromString(mask);
   wifiManager.setSTAStaticIPConfig(_ip, _gw, _mask);
  }
  while(wifiManager.startConfigPortal(WIFI_SETUP_AP)) {
    Serial.print("Connected to ");
    Serial.println(WiFi.SSID());
    event.wifiConnected++;
  } 
  //if you get here you have connected to the WiFi
  Serial.println("connected...yeey :)");
  RUN_DELETE;
}

uint32_t printTime() {
  Serial.println((uint32_t)getTime());
  return 10000;  
}
// Query Reset Key ststus change and flag events
uint32_t checkKey() {
  if (digitalRead(RESET_PIN) == HIGH && !status.keyPressed) {
    event.keyPressed++;
    status.keyPressed = true;
  }
  if (digitalRead(RESET_PIN) == LOW && status.keyPressed) {
    event.keyReleased++;
    status.keyPressed = false;
  }
  return 100;
}
#define KEY_LONG_TIME 3000
// Called on Key Pressed Event
uint32_t keyPressed() {
  digitalWrite(D0, LOW);
  taskAddWithDelay(keyLongPressed, KEY_LONG_TIME);
  return RUN_NEVER;
}
// Called on Key Release Event
uint32_t keyReleased() {
  digitalWrite(D0, HIGH);
  taskDel(keyLongPressed);
  return RUN_NEVER;
}
// Called if Key Pressed for KEY_LONG_TIME mS
uint32_t keyLongPressed() {
  wifiManager();
  return RUN_DELETE;
}

void setup(){
  pinMode(D0, OUTPUT);    //For debug
  digitalWrite(D0, HIGH); //For debug

  Serial.begin(74880);    //For debug
  taskAdd(wifiStart);     // Add task with Wi-Fi initialization code
  taskAdd(initRTC);       // Add task with RTC init
  taskAddWithSemaphore(initNTP, &event.wifiConnected); // Run initNTP() on Wi-Fi connection
  taskAdd(printTime);     //For debug
  taskAdd(checkKey);      // Key query
  taskAddWithSemaphore(keyPressed, &event.keyPressed); // Run keyPressed() on keyPressed event
  taskAddWithSemaphore(keyReleased, &event.keyReleased); // Run keyReleased() on keyRelease event
}
void loop(void){
  taskExec();
  yield();
}
