#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <WiFiManager.h>
#define RUN_TASKS 32
#include <Run.h>

#define VERSION "0.5.2"

// Pin to activete WiFiManager configuration routine
#define RESET_PIN D8
// Current query interval (mS)
#define A0_DELAY 1000

struct events {
 uint16_t wifiConnected		= 0;
 uint16_t resetLongPress 	= 0;
 uint16_t resetShortPress	= 0;
 uint16_t turnOffAllSockets= 0;
 uint16_t keyPressed      = 0;
 uint16_t keyReleased     = 0;
 uint16_t saveParams      = 0;
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

#define PUMP_NONE         "100"
#define PUMP_SINGLE       "110"
#define PUMP_DOUBLE_PULSE "121"
#define PUMP_DOUBLE_ALT   "122"
#define PUMP_DOUBLE_SER   "123"
#define PUMP_DOUBLE_RND   "124"
#define PUMP_QUAD_PULSE   "141"
#define PUMP_QUAD_ALT     "142"
#define PUMP_QUAD_SER     "143"
#define PUMP_QUAD_RND     "144"

#define GROUP_HTML_BASE 11
/*
bool   dhcp = true;
String ip   = "192.168.20.99";
String mask = "255.255.255.0";
String gw   = "192.168.20.2";
String dns  = "192.168.20.2";
*/
String ntp1 = "192.168.30.30";
String ntp2 = "192.168.30.4";
String ntp3 = "pool.ntp.org";
String tz   = "5";
String admin = "admin";
String pass = "password";
float amps = 0;     // Current value from A0
String name = "socket";
//Select one of following AC current read implementations
// ---------------------------
 #include "WiFiACSimple.h"
 //#include "WiFiCurrent.h"
 //#include "WiFiACRMS.h"
// ---------------------------
#include "WiFiTime.h"
#include "WiFiControl.h"
#include "WiFiConfig.h"
#include "WiFiWeb.h"
#include "discovery.h"
#include "update.h"

#define WIFI_SETUP_AP "AutoConnectAP"
#define WIFI_CHECK_DELAY 1000
#define WIFI_CHECK_COUNT 5

uint32_t wifiStart() {
  WiFi.mode(WIFI_STA);
/*
  if (!dhcp) {
   IPAddress _ip, _gw, _mask, _dns;
   _ip.fromString(ip);
   _gw.fromString(gw);
   _mask.fromString(mask);
   _dns.fromString(dns);
   WiFi.config(_ip, _gw, _mask, _dns);
  }
*/
  WiFi.begin();
  //Serial.print("Connecting to ");
  //Serial.println(WiFi.SSID());
  taskAddWithDelay(wifiWait, WIFI_CHECK_DELAY);
  return RUN_DELETE;
}
uint8_t waitWF = 1;
uint32_t wifiWait() {
  if(WiFi.status() != WL_CONNECTED) {
    if (waitWF <= WIFI_CHECK_COUNT) {
      //Serial.println("Waiting Wi-Fi");
      if (waitWF > 0) {
        waitWF++;
      }
      return WIFI_CHECK_DELAY;
    } else {
      WiFi.mode(WIFI_OFF);
      taskAddWithDelay(wifiStart, WIFI_CHECK_DELAY);
    }
  } else {
    //Serial.println(WiFi.localIP());
    event.wifiConnected++;
    //Serial.println(WiFi.localIP().toString());
  }
  return RUN_DELETE;
}
void cbSaveParams() {
  event.saveParams++;
}
void cbConnected(WiFiManager *wfm) {
}

uint32_t wifiManager() {
  server.stop();
  WiFiManager wifiManager;
  WiFi.mode(WIFI_OFF);
  delay(1000);
  wifiManager.setSaveConfigCallback(cbSaveParams);
  //wifiManager.resetSettings();
  wifiManager.setBreakAfterConfig(true);
  wifiManager.setTimeout(120);
  /*
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
  */
  wifiManager.addParameter(&pName);
  //wifiManager.setConnectTimeout(WIFI_CHECK_DELAY * WIFI_CHECK_COUNT);
  /*
  if (!dhcp) {
   IPAddress _ip, _gw, _mask;
   _ip.fromString(ip);
   _gw.fromString(gw);
   _mask.fromString(mask);
   wifiManager.setSTAStaticIPConfig(_ip, _gw, _mask);
  }
  */
  //while(
  //  wifiManager.autoConnect()) {
    wifiManager.startConfigPortal(WIFI_SETUP_AP);//) {
  //  Serial.print("Connected to ");
  //  Serial.println(WiFi.SSID());
    //event.wifiConnected++;
  //} 
  //if you get here you have connected to the WiFi
  Serial.println("config done");
  if (event.saveParams > 0) {
     saveConfig();
     Serial.println("save done");
     delay(1000);
     ESP.restart();
  }
  RUN_DELETE;
}

uint32_t printTime() {
  //Serial.println((uint32_t)getTime());
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
  taskAddWithDelay(keyLongPressed, KEY_LONG_TIME);
  return RUN_NEVER;
}
// Called on Key Release Event
uint32_t keyReleased() {
  taskDel(keyLongPressed);
  ESP.reset();
  return RUN_NEVER;
}
// Called if Key Pressed for KEY_LONG_TIME mS
uint32_t keyLongPressed() {
  //digitalWrite(D0, HIGH);
  taskDel(keyReleased);
  turnOffAllSockets();
  wifiManager();
  //digitalWrite(D0, LOW);
  return RUN_DELETE;
}
uint32_t initDbg() {
  //socket[2]->na();
  //socket[2]->wave = &wave;
  //feedSchedule.schedule1.act = true;
  //feedSchedule.schedule1.on = 8*60;
  //feedSchedule.schedule1.off = 8*60+20;
  //socket[2]->feedOverride = SON;
   // socket[2]->setGroup(&group[0]);
  //taskAddWithDelay(initDbg2, 15000);
  return RUN_DELETE;
}
uint32_t initDbg2() {
  //socket[2]->off(15);
  return RUN_DELETE;
}

void setup() {
  //pinMode(D0, OUTPUT);    //For debug
  //digitalWrite(D0, HIGH); //For debug
  Serial.begin(74880);    //For debug
  SPIFFS.begin();
  xml.init((uint8_t *)buffer, sizeof(buffer), &XML_callback);
  readConfig();
  //readState();
  taskAdd(wifiStart);     // Add task with Wi-Fi initialization code
  taskAdd(initRTC);       // Add task with RTC init
  taskAdd(initSockets);   // Add task to initilize Sockets control
  taskAdd(initA0);        // Add task to initialize ADC query
  //taskAdd(initDbg);
  taskAddWithSemaphore(initNTP, &event.wifiConnected);  // Run initNTP() on Wi-Fi connection
  taskAddWithSemaphore(initWeb, &event.wifiConnected);  // Run initWeb() on Wi-Fi connection
  taskAddWithSemaphore(discovery, &event.wifiConnected);
  taskAddWithSemaphore(initUpdate, &event.wifiConnected);
  //taskAdd(printTime);     //For debug
  taskAdd(checkKey);      // Key query
  taskAddWithSemaphore(keyPressed, &event.keyPressed);  // Run keyPressed() on keyPressed event
  taskAddWithSemaphore(keyReleased, &event.keyReleased);// Run keyReleased() on keyRelease event
  //taskAddWithSemaphore(saveConfig, &event.saveParams);   // Save config on WiFiManager request
  taskAdd(readState);
  taskAdd(queryA0);
//  inputStats.setWindowSecs(windowLength);
}
void loop(void) {
  taskExec();
  yield();
}
