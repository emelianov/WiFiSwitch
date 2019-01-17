#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <WiFiManager.h>
#define RUN_TASKS 32
#include <Run.h>
#include "settings.h"

ADC_MODE(ADC_VCC);

#define VERSION "0.7.0"

// Pin to activete WiFiManager configuration routine
#define RESET_PIN D8

// Current query interval (mS)
#define A0_DELAY 1000

#define WIFI_SETUP_AP "SocketWIFI_AP_"
#define WIFI_CHECK_DELAY 1000
#define WIFI_CHECK_COUNT 150

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
String ip   = "192.168.30.118";
String mask = "255.255.255.0";
String gw   = "192.168.30.4";
String dns  = "192.168.30.4";
*/
String ntp1 = "pool.ntp.org";
String ntp2 = "time.nist.gov";
String ntp3 = "time.apple.com";
String tz   = "5";
String admin = "admin";
String pass = "password";
float amps = 0;     // Current value from A0
String name = "socket";
uint32_t wifiStart();

#include "ACemon.h"
#include "clock.h"
#include "control.h"
#include "config.h"
#include "web.h"
#include "discovery.h"
#include "update.h"
#include "ping.h"

uint8_t waitWF;
uint32_t wifiStart() {
  WiFi.mode(WIFI_OFF);
  delay(100);
  WiFi.mode(WIFI_STA);
  WiFi.begin();
  waitWF = 1;
  taskAddWithDelay(wifiWait, WIFI_CHECK_DELAY);
 #ifdef WFS_DEBUG
  Serial.print("Connecting to ");
  Serial.println(WiFi.SSID());
 #endif
  return RUN_DELETE;
}

uint32_t wifiWait() {
  if(WiFi.status() != WL_CONNECTED) {
    if (waitWF <= WIFI_CHECK_COUNT) {
     #ifdef WFS_DEBUG
      Serial.print(".");
     #endif
      //if (waitWF > 0) {
        waitWF++;
      //}
      return WIFI_CHECK_DELAY;
    } else {
      waitWF = 1;
      //WiFi.mode(WIFI_OFF);
      taskAddWithDelay(wifiStart, WIFI_CHECK_DELAY);
      return RUN_DELETE;
    }
  } else {
   #ifdef WFS_DEBUG
    Serial.println(WiFi.localIP().toString());
   #endif
    event.wifiConnected++;
    randomSeed(millis());
    taskAdd(initPing);
  }
  return RUN_DELETE;
}
void cbSaveParams() {
  event.saveParams++;
}
void cbConnected(WiFiManager *wfm) {
}

uint32_t restartESP() {
  timer1_disable();
  ESP.restart();
  return RUN_DELETE;
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
  wifiManager.addParameter(&pNameT);
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
    char apname[sizeof(WIFI_SETUP_AP)+5];
    byte mac[6];
    WiFi.macAddress(mac);
    sprintf(apname, "%s%02X%02X", WIFI_SETUP_AP, mac[4], mac[5]);
    wifiManager.startConfigPortal(apname);
    //wifiManager.startConfigPortal(WIFI_SETUP_AP);//) {
   #ifdef WFS_DEBUG
    Serial.print("Connected to ");
    Serial.println(WiFi.SSID());
   #endif
    //event.wifiConnected++;
  //} 
  //if you get here you have connected to the WiFi
 #ifdef WFS_DEBUG
  Serial.println("config done");
 #endif
  if (event.saveParams > 0) {
     name = pName.getValue();
     saveConfig();
     //Serial.println("save done");
     delay(1000);
     ESP.restart();
  }
  RUN_DELETE;
}

uint32_t printTime() {
  Serial.println(getTime());
  return 1000;  
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
  taskDel(keyReleased);
  turnOffAllSockets();
  wifiManager();
  return RUN_DELETE;
}

void setup() {
 #ifdef WFS_DEBUG
  Serial.begin(74880);    //For debug
 #endif
  SPIFFS.begin();
  //xmlo.init((uint8_t *)buffer, sizeof(buffer), &XML_callback);
  readConfig();
  //readState();
  taskAdd(wifiStart);     // Add task with Wi-Fi initialization code
  taskAdd(initRTC);       // Add task with RTC init
  taskAdd(initSockets);   // Add task to initilize Sockets control
  taskAddWithDelay(initA0, 5000);        // Add task to initialize ADC query
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
  //taskAdd(initPing);
}
void loop(void) {
  //wdt_enable(0);
  taskExec();
  yield();
  wdt_reset();
}
