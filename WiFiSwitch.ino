#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <WiFiManager.h>
#include <Run.h>

struct events {
 uint8_t wifiConnected		= 0;
 uint8_t resetLongPress 	= 0;
 uint8_t resetShortPress	= 0;
 uint8_t turnOffAllSockets= 0;
};
struct statuses {
 bool wifiConnected	= false;
 bool ntpSync			  = false;
 bool rtcPresent		= false;
 bool rtcValid			= false;
};
events event;
statuses status;
bool   dhcp = true;
String ip   = "192.168.20.99";
String mask = "255.255.255.0";
String gw   = "192.168.20.2";
String dns  = "192.168.20.2";
String ntp1 = "192.168.20.2";
String ntp2 = "192.168.20.11";
String ntp3 = "pool1.ntp.org";
String tz   = "0";

#define WIFI_SETUP_AP "AutoConnectAP"
#define WIFI_CHECK_DELAY 1000
#define WIFI_CHECK_COUNT 5
uint32_t wiFiStart() {
  WiFi.mode(WIFI_STA);
  WiFi.begin();
  Serial.print("Connecting ");
  Serial.print(WiFi.SSID());
  taskAddWithDelay(wiFiWait, WIFI_CHECK_DELAY);
  return RUN_DELETE;
}
uint8_t waitWF = 0;
uint32_t wiFiWait() {
  if(WiFi.status() != WL_CONNECTED) {
    if (waitWF <= WIFI_CHECK_COUNT) {
      Serial.println("Waiting Wi-Fi");
      if (waitWF > 0) {
        waitWF++;
      }
      return WIFI_CHECK_DELAY;
    }
  } else {
    event.wifiConnected++;
  }
  return RUN_DELETE;
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
  while(!wifiManager.autoConnect(WIFI_SETUP_AP)) {
    Serial.println("failed to connect and hit timeout");
    delay(5000);
  } 
  }
  //if you get here you have connected to the WiFi
  Serial.println("connected...yeey :)");
  RUN_DELETE;
}
void setup(){
  Serial.begin(74880);
  //uint8_t waitWF = 0;
  //WiFi.mode(WIFI_STA);
  //WiFi.begin();
  //Serial.print("Connecting");
  //while(WiFi.status() != WL_CONNECTED && waitWF < 6) {
  //  Serial.print(".");
  //  waitWF++;
  //  delay(1000);
  //}
  //Serial.println();
  //Serial.println(WiFi.SSID());
  if (waitWF > 5) {
  String ip = "192.168.20.99";
  String mask = "255.255.255.0";
  String gw = "192.168.20.2";
  String dns = "192.168.20.2";
  String ntp1 = "192.168.20.2";
  String ntp2 = "192.168.20.11";
  String ntp3 = "pool1.ntp.org";
  String tz = "0";
  WiFiManagerParameter pNet("IP/Mask/Gw/DNS");
  WiFiManagerParameter pIp("ip", "IP", ip.c_str(), 15);
  WiFiManagerParameter pMask("mask", "Network mask", mask.c_str(), 15);
  WiFiManagerParameter pGw("gw", "Default gateway", gw.c_str(), 15);
  WiFiManagerParameter pDns("dns", "DNS", dns.c_str(), 15);
  WiFiManagerParameter pNtp("NTP Servers");
  WiFiManagerParameter pNtp1("ntp1", "NTP server", ntp1.c_str(), ntp1.length());
  WiFiManagerParameter pNtp2("ntp2", "NTP server", ntp2.c_str(), ntp2.length());
  WiFiManagerParameter pNtp3("ntp3", "NTP server", ntp3.c_str(), ntp3.length());
  WiFiManagerParameter pTz("tz", "Time Zone", tz.c_str(), 4, "TimeZone");
  
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
  while(!wifiManager.autoConnect("AutoConnectAP")) {
    Serial.println("failed to connect and hit timeout");
    delay(5000);
  } 
  }
  //if you get here you have connected to the WiFi
  Serial.println("connected...yeey :)");

}
void loop(void){
  taskExec();
  yield();
}
