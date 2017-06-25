#pragma once
#include <FS.h>
#include <TinyXML.h>
#define CFG "/config.xml"
#define STATE "/state.xml"

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

//Format 00:00PM
time_t strToTime(String tm) {
  if (tm.length() >= 7) {
    Serial.println(tm.substring(0,2).toInt()*3600 + tm.substring(3,5).toInt()*60 + (tm.substring(5,7)=="PM")?12*3600:0);
    Serial.println(tm.substring(0,2).toInt());
    Serial.println(tm.substring(3,5).toInt());
    return (tm.substring(0,2).toInt()*3600L + tm.substring(3,5).toInt()*60L) + ((tm.substring(5,7)=="PM")?12*3600L:0);
  } else {
    return 0;
  }
}
//Format 00:00
time_t strToTime24(String tm) {
  if (tm.length() >= 5) {
    return (tm.substring(0,2).toInt()*3600L + tm.substring(3,5).toInt()*60L);
  } else {
    return 0;
  }
}
String timeToStr(time_t t) {
    String ampm = "AM";
    char  strTime[10];
    uint16_t minutesFromMidnight = t % 86400UL / 60;
    if (minutesFromMidnight >= 720) {
      ampm = "PM";
      if (minutesFromMidnight >= 780)
        minutesFromMidnight -= 720;
    }
    sprintf_P(strTime, PSTR("%02d:%02d%s"), (uint8_t)(minutesFromMidnight / 60), (uint8_t)(minutesFromMidnight % 60), ampm.c_str());
    return String(strTime);
}
String timeToStr24(time_t t) {
    char  strTime[10];
    uint16_t minutesFromMidnight = t % 86400UL / 60;
    sprintf_P(strTime, PSTR("%02d:%02d"), (uint8_t)(minutesFromMidnight / 60), (uint8_t)(minutesFromMidnight % 60));
    return String(strTime);
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

uint32_t saveState() {
   uint8_t i;
   File configFile = SPIFFS.open(F(STATE), "w");
   if (configFile) {
    char buf[400];
    sprintf_P(buf, PSTR("<?xml version = \"1.0\" ?>\n<state>\n"));
    configFile.write((uint8_t*)buf, strlen(buf));
      sprintf_P(buf, PSTR("<Switch>%s</Switch><Override>%lu</Override><Waiting>%s</Waiting>"),
              (feed->mode==SON)?"on":(feed->mode==SOFF)?"off":"default",
              taskRemainder(feedTask)/1000,
              (feed->modeWaiting==SON)?"on":(feed->modeWaiting==SOFF)?"off":"default"
              );
    configFile.write((uint8_t*)buf, strlen(buf));
    for (i = 0; i < GROUP_COUNT; i++) {
      sprintf_P(buf, PSTR("<Switch>%s</Switch><Override>%lu</Override><Waiting>%s</Waiting>"),
              (group[i]->mode==SON)?"on":(group[i]->mode==SOFF)?"off":"default",
              taskRemainder(groupOverride[i])/1000,
              (group[i]->modeWaiting==SON)?"on":(group[i]->modeWaiting==SOFF)?"off":"default"
              );
      configFile.write((uint8_t*)buf, strlen(buf));
    }
  // Socket override timers state and group
//  #define GROUP_HTML_BASE 11
  for (i = 0; i < SOCKET_COUNT; i++) {
    uint8_t gr = 0;
    for (uint8_t j = 0; j < GROUP_COUNT; j++) {
      if (socket[i]->group == group[j]) {
        gr = j + GROUP_HTML_BASE;
        continue; 
      }
    }
    sprintf_P(buf, PSTR("<Socket>%s</Socket>\n\
<TimerCheckbox>%s</TimerCheckbox>\n<TimerCheckbox>%s</TimerCheckbox>\n<TimerValue>%s</TimerValue>\n<TimerValue>%s</TimerValue>\n<TimerValue>%s</TimerValue>\n<TimerValue>%s</TimerValue>\n\
<Group>%d</Group>\n<Override>%lu</Override><Switch>%s</Switch><Waiting>%s</Waiting><FSwitch>%s</FSwitch>"),
              socket[i]->isOn()?"checked":"unckecked",
              socket[i]->schedule1.active()?"checked":"unckecked",
              socket[i]->schedule2.active()?"checked":"unckecked",
              timeToStr(socket[i]->schedule1.on).c_str(),
              timeToStr(socket[i]->schedule1.off).c_str(),
              timeToStr(socket[i]->schedule2.on).c_str(),
              timeToStr(socket[i]->schedule2.off).c_str(),
              gr,
              taskRemainder(socketTasks[i]) / 1000,
              (socket[i]->mode==SON)?"on":(socket[i]->mode==SOFF)?"off":"default",
              (socket[i]->modeWaiting==SON)?"on":(socket[i]->modeWaiting==SOFF)?"off":"default",
              (socket[i]->feedOverride==SON)?"on":(socket[i]->feedOverride==SOFF)?"off":"default"
              );
          configFile.write((uint8_t*)buf, strlen(buf));
    }
    sprintf_P(buf, PSTR("</config>\n"));
    configFile.write((uint8_t*)buf, strlen(buf));

    configFile.close();
   }
   return RUN_DELETE;    
}

