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
uint8_t buffer[300];
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
    //Serial.println(tm.substring(0,2).toInt()*3600 + tm.substring(3,5).toInt()*60 + (tm.substring(5,7)=="PM")?12*3600:0);
    //Serial.println(tm.substring(0,2).toInt());
    //Serial.println(tm.substring(3,5).toInt());
    uint8_t hh = tm.substring(0,2).toInt();
    return ((hh==12?0:hh)*3600L + tm.substring(3,5).toInt()*60L) + (tm.substring(5,7)=="PM"?12*3600L:0);
  } else {
    return 0;
  }
}
//Format 00:00
time_t strToTime24(String tm) {
  if (tm.length() >= 5) {
    //return (tm.substring(0,2).toInt()*3600L + tm.substring(3,5).toInt()*60L);
    return (tm.substring(0,2).toInt()*60L + tm.substring(3,5).toInt());
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
//      if (minutesFromMidnight >= 720)
        minutesFromMidnight -= 720;
    }
    uint8_t hh = (uint8_t)(minutesFromMidnight / 60);
    sprintf_P(strTime, PSTR("%02d:%02d%s"), (hh == 0)?12:hh, (uint8_t)(minutesFromMidnight % 60), ampm.c_str());
    return String(strTime);
}
String timeToStr24(time_t t) {
    char  strTime[10];
    uint16_t minutesFromMidnight = t % 86400UL;
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
        if (xmlData.length() > 7) {
            dhcp = false;
            ip = xmlData;
        }
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
    char buf[512];
    sprintf_P(buf, PSTR("<?xml version = \"1.0\" ?>\n<state>\n"));
    configFile.write((uint8_t*)buf, strlen(buf));
    sprintf_P(buf, PSTR("<FSwitch>%s</FSwitch><FOverride>%lu</FOverride><FWaiting>%s</FWaiting>"),
              (feed->mode==SON)?"on":(feed->mode==SOFF)?"off":"default",
              taskRemainder(feedTask)/1000,
              (feed->modeWaiting==SON)?"on":(feed->modeWaiting==SOFF)?"off":"default"
              );
    configFile.write((uint8_t*)buf, strlen(buf));
    for (i = 0; i < GROUP_COUNT; i++) {
      sprintf_P(buf, PSTR("<GSwitch>%s</GSwitch><GOverride>%lu</GOverride><GWaiting>%s</GWaiting>"),
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
    sprintf_P(buf, PSTR("<Manual>%s</Manual><Socket>%s</Socket>\n\
<TimerCheckbox1>%s</TimerCheckbox1>\n<TimerCheckbox2>%s</TimerCheckbox2>\n<TimerValue1on>%s</TimerValue1on>\n<TimerValue1off>%s</TimerValue1off>\n\
<TimerValue2on>%s</TimerValue2on>\n<TimerValue2off>%s</TimerValue2off>\n\
<Group>%d</Group>\n<Override>%lu</Override><Switch>%s</Switch><Waiting>%s</Waiting><SwitchF>%s</SwitchF><name>%s</name>"),
              (socket[i]->manual == SON)?"checked":"unchecked",
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
              (socket[i]->feedOverride==SON)?"on":(socket[i]->feedOverride==SOFF)?"off":"default",
              socket[i]->name.c_str()
              );
      configFile.write((uint8_t*)buf, strlen(buf));
    }
    sprintf_P(buf, PSTR("<FTimerCheckbox1>%s</FTimerCheckbox1>\n<FTimerCheckbox2>%s</FTimerCheckbox2>\n<FTimerValue1on>%s</FTimerValue1on>\n<FTimerValue1off>%s</FTimerValue1off>\n<FTimerValue2on>%s</FTimerValue2on>\n<FTimerValue2off>%s</FTimerValue2off>\n"),
              feedSchedule.schedule1.active()?"checked":"unckecked",
              feedSchedule.schedule2.active()?"checked":"unckecked",
              timeToStr(feedSchedule.schedule1.on).c_str(),
              timeToStr(feedSchedule.schedule1.off).c_str(),
              timeToStr(feedSchedule.schedule2.on).c_str(),
              timeToStr(feedSchedule.schedule2.off).c_str()
              );
    configFile.write((uint8_t*)buf, strlen(buf));
    sprintf_P(buf, PSTR("<pump>%s</pump><wave>%s</wave></state>\n"),
              pump.c_str(), timeToStr24(wave.period).c_str());
    configFile.write((uint8_t*)buf, strlen(buf));

    configFile.close();
   }
   return RUN_DELETE;    
}
uint32_t readState() {
  xml.reset();
  xmlTag = "";
  xmlOpen = "";
  File configFile = SPIFFS.open(F(STATE), "r");
  if (configFile) {
   char c;
   uint8_t m = 0;
   uint8_t t1s = 0;
   uint8_t t2s = 0;
   uint8_t t1on = 0;
   uint8_t t1off = 0;
   uint8_t t2on = 0;
   uint8_t t2off = 0;
   uint8_t g = 0;
   uint8_t f = 0;
   uint8_t n = 0;
   while (configFile.read((uint8_t*)&c, 1) == 1) {
    xml.processChar(c);
    if (xmlTag != "") {
       if
      (xmlTag.endsWith(F("/Manual"))) {
        socket[m]->manual = (xmlData == "checked")?SON:SOFF;
        if (m < SOCKET_COUNT - 1) m++;
       } else if 
      (xmlTag.endsWith(F("/TimerCheckbox1"))) {
        socket[t1s]->schedule1.act = (xmlData == "checked");
        if (t1s < SOCKET_COUNT - 1) t1s++;
       } else if 
      (xmlTag.endsWith(F("/TimerCheckbox2"))) {
        socket[t2s]->schedule2.act = (xmlData == "checked");
        if (t2s < SOCKET_COUNT - 1) t2s++;
       } else if 
      (xmlTag.endsWith(F("/TimerValue1on"))) {
        socket[t1on]->schedule1.on = strToTime(xmlData);
        if (t1on < SOCKET_COUNT - 1) t1on++;
       } else if 
      (xmlTag.endsWith(F("/TimerValue1off"))) {
        socket[t1off]->schedule1.off = strToTime(xmlData);
        if (t1off < SOCKET_COUNT - 1) t1off++;
       } else if 
      (xmlTag.endsWith(F("/TimerValue2on"))) {
        socket[t2on]->schedule2.on = strToTime(xmlData);
        if (t2on < SOCKET_COUNT - 1) t2on++;
       }  else if 
      (xmlTag.endsWith(F("/Timervalue2off"))) {
        socket[t2off]->schedule2.off = strToTime(xmlData);
        if (t2off < SOCKET_COUNT - 1) t2off++;
       } else if 
      (xmlTag.endsWith(F("/Group"))) {
        uint8_t _g = xmlData.toInt() - GROUP_HTML_BASE;
        //Serial.println(_g);
        if (_g >= 0 && _g < GROUP_COUNT) socket[g]->setGroup(group[_g]);
        if (g < SOCKET_COUNT - 1) g++;
       } else if 
      (xmlTag.endsWith(F("/SwitchF"))) {
        if (xmlData == "on") socket[f]->feedOverride = SON;
        if (xmlData == "off") socket[f]->feedOverride = SOFF;
        if (f < SOCKET_COUNT - 1) f++;
       } else if 
      (xmlTag.endsWith(F("/name"))) {
        socket[n]->name = xmlData;
        if (n < SOCKET_COUNT - 1) n++;
       } else if 
      (xmlTag.endsWith(F("/wave"))) {
        time_t t = strToTime24(xmlData);
        //Serial.print(t);
        //if (wave.period != t)
          wave.on(t);
      } else if 
      (xmlTag.endsWith(F("/pump"))) {
        //if (pump != xmlData) {
          setPump(xmlData);
          //Serial.print("Pump: ");
          //Serial.println(wave.period);
          wave.on(wave.period);
        //}
       } else if 
      (xmlTag.endsWith(F("/FTimerCheckbox1"))) {
        feedSchedule.schedule1.act = (xmlData == "checked");
       } else if 
      (xmlTag.endsWith(F("/FTimerCheckbox2"))) {
        feedSchedule.schedule2.act = (xmlData == "checked");
       } else if 
      (xmlTag.endsWith(F("/FTimerValue1on"))) {
        feedSchedule.schedule1.on = strToTime(xmlData);
       } else if 
      (xmlTag.endsWith(F("/FTimerValue1off"))) {
        feedSchedule.schedule1.off = strToTime(xmlData);
       } else if 
      (xmlTag.endsWith(F("/FTimerValue2on"))) {
        feedSchedule.schedule2.on = strToTime(xmlData);
       }  else if 
      (xmlTag.endsWith(F("/FTimervalue2off"))) {
        feedSchedule.schedule2.off = strToTime(xmlData);
       }
      xmlTag = "";
      xmlData = "";
    }
   }
   configFile.close();
  }
  return RUN_DELETE;
}

