#pragma once
#include <FS.h>
#include <TinyXML.h>
#define CFG "/config.xml"
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

bool   dhcp = true;
String ip   = "192.168.20.99";
String mask = "255.255.255.0";
String gw   = "192.168.20.2";
String dns  = "192.168.20.2";
String ntp1 = "192.168.30.30";
String ntp2 = "192.168.30.4";
String ntp3 = "pool1.ntp.org";
String tz   = "5";
String admin = "admin";
String pass = "password";

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


//XML processor settings
String xmlOpen;
String xmlTag;
String xmlData;
String xmlAttrib;
TinyXML xml;
uint8_t buffer[150];
void XML_callback(uint8_t statusflags, char* tagName, uint16_t tagNameLen, char* data, uint16_t dataLen) {
  if
  (statusflags & STATUS_TAG_TEXT) {
    xmlTag = String(tagName);
    xmlData = String(data);
  } else if
  (statusflags & STATUS_START_TAG) {
    xmlOpen = String(tagName);
  }
}

uint32_t readConfig() {
  xml.reset();
  xmlTag = "";
  xmlOpen = "";
  File configFile = SPIFFS.open(F(CFG), "r");
  if (configFile) {
   char c;
   while (configFile.read((uint8_t*)&c, 1) == 1) {
    xml.processChar(c);
    if (xmlTag != "") {
       if 
      (xmlTag.endsWith(F("/admin"))) {
        admin = xmlData;
       } else if 
      (xmlTag.endsWith(F("/adminpass"))) {
        pass = xmlData;
       } else if 
      (xmlTag.endsWith(F("/ip"))) {
        dhcp = false;
        ip = xmlData;
       } else if 
      (xmlTag.endsWith(F("/mask"))) {
        mask = xmlData;
       } else if 
      (xmlTag.endsWith(F("/gw"))) {
        gw = xmlData;
       }  else if 
      (xmlTag.endsWith(F("/dns"))) {
        dns = xmlData;
       } else if 
      (xmlTag.endsWith(F("/ntp1"))) {
        ntp1 = xmlData;
       } else if 
      (xmlTag.endsWith(F("/ntp2"))) {
        ntp2 = xmlData;
       } else if 
      (xmlTag.endsWith(F("/ntp3"))) {
        ntp3 = xmlData;
       } else if 
      (xmlTag.endsWith(F("/timezone"))) {
        tz = xmlData.toInt();
       }
      xmlTag = "";
      xmlData = "";
    }
   }
   configFile.close();
  }
  return RUN_DELETE;
}

uint32_t saveConfig() {
   ip = pIp.getValue();
   mask = pMask.getValue();
   gw = pGw.getValue();
   dns = pDns.getValue();
   ntp1 = pNtp1.getValue();
   ntp2 = pNtp2.getValue();
   ntp3 = pNtp3.getValue();
   tz = pTz.getValue();
   File configFile = SPIFFS.open(F(CFG), "w");
   if (configFile) {
    char buf[400];
    sprintf_P(buf, PSTR("<?xml version = \"1.0\" ?>\n<config>\n"));
    configFile.write((uint8_t*)buf, strlen(buf));
    if (ip.length() >= 8) {
      sprintf_P(buf, PSTR("<ip>%s</ip><mask>%s</mask>\n<gw>%s</gw>\n<dns>%s</dns>"), ip.c_str(), mask.c_str(), gw.c_str(), dns.c_str());
      configFile.write((uint8_t*)buf, strlen(buf));
    }
    sprintf_P(buf, PSTR("<ntp1>%s</ntp1>\n<ntp2>%s</ntp2>\n<ntp3>%s</ntp3><timezone>%d</timezone></config>"), ntp1.c_str(), ntp2.c_str(), ntp3.c_str(), tz.toInt());
    configFile.write((uint8_t*)buf, strlen(buf));
    configFile.close();
   }
   return RUN_DELETE;  
}
