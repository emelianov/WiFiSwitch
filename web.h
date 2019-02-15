#pragma once
#include <detail/RequestHandlersImpl.h>
#include "settings.h"

#define INDEX "index.html"
#define UPLOADUSER  "admin"
#define UPLOADPASS "password"
#undef UPLOADPASS
#define BUSY ;
#define IDLE ;
#define SAVE_DELAY 5000
#define SAVE_SAMPLES 100

ESP8266WebServer server(80);      // create a server at port 80
uint32_t sequence = 0;
#ifdef WFS_DEBUG
void handleDebug() {
  uint16_t _timeout = 500;
  DateTime now = rtc.now();
  IPAddress n1, n2, n3;
  if (!WiFi.hostByName(ntp1.c_str(), n1, _timeout)) {
    n1 =IPAddress(0,0,0,0);
  }
  if (!WiFi.hostByName(ntp2.c_str(), n2, _timeout)) {
    n2 =IPAddress(0,0,0,0);
  }
  if (!WiFi.hostByName(ntp3.c_str(), n3, _timeout)) {
    n3 =IPAddress(0,0,0,0);
  }
  sprintf_P(data,\
          PSTR("<?xml version = \"1.0\" encoding=\"UTF-8\" ?><private><heap>%d</heap><rssi>%d</rssi><uptime>%ld</uptime>\
          <rtcinit>%s</rtcinit><rtcpower>%s</rtcpower><rtc>%ld</rtc><ntp>%ld</ntp><sys>%ld</sys><s1>%s</s1><s2>%s</s2><s3>%s</s3><n1>%s</n1><n2>%s</n2><n3>%s</n3></private>"),\
          ESP.getFreeHeap(), WiFi.RSSI(), (uint32_t)millis()/1000,\
          (!status.rtcPresent)?"Failed":"Ok", rtc.lostPower()?"Failed":"Ok", now.unixtime(), time(NULL), getTime(),\
          ntp1.c_str(), ntp2.c_str(), ntp3.c_str(), (n1.toString()).c_str(), (n2.toString()).c_str(), (n3.toString()).c_str()\
          );
  server.sendHeader(F("Connection"), F("close"));
  server.sendHeader(F("Cache-Control"), F("no-store, must-revalidate"));
  server.send(200, "text/xml", data);  
}
#endif

uint32_t restartESP();
String swState(OverrideMode o) {
  return (o==SON)?"on":(o==SOFF)?"off":"default";
}
// callback function that is called by Web server in case if /ajax_input?...
void ajaxInputs() {
  uint8_t i;
  char* xmlBuffer = data + (SAVE_SAMPLES * sizeof(int16_t) * 2);
  char* p = xmlBuffer;
  //char  strTime[4][8];    // Schedule time strings buffer
  //uint16_t  minutesFromMidnight;
  bool save = false;
  // Check if got Socket on/off switching or Schedule changes
  {
    String sched1;
    String sched2;
    String tm11;
    String tm12;
    String tm13;
    String tm14;
    String soc;
    String cArg;
    String tArg;
    String nArg;
    String v;
    
    for (i = 0; i < SOCKET_COUNT; i++) {
      soc = F("SOC");
      soc += String(i);
      if (server.hasArg(soc)) {
        if (server.arg(soc) == "1") {
          //socket[i]->on();
          socket[i]->manual = SON;
        } else {
          socket[i]->manual = SOFF;
          //socket[i]->na();
        }
      }
      sched1 = F("TCB");
      sched1 += String(i*2);  // e.g. ?TCB2=1
      sched2 = F("TCB");
      sched2 += String(i*2+1);  // e.g. ?TCB3=1
      tm11   =  F("TIM");
      tm11   += String(i*4);    // e.g. ?TIM8=10:15AM
      tm12   =  F("TIM");
      tm12   += String(i*4+1);  // e.g. ?TIM9=11:00PM
      tm13   = F("TIM");
      tm13   += String(i*4+2);  // e.g. ?TIM10=12:00PM
      tm14   = F("TIM");
      tm14   += String(i*4+3);  // e.g. ?TIM11=21:45PM
      if (server.hasArg(sched1)) {          // First Schedule switch updateed
        if (server.arg(sched1) == "1") {    // Switch is turned On 
          socket[i]->schedule1.act=true;
        } else {                            // Switch is turned Off
          socket[i]->schedule1.act=false;
        }
        save = true;
      }
      if (server.hasArg(sched2)) {        // Second Schedule Switch updated
        if (server.arg(sched2) == "1") {
          socket[i]->schedule2.act=true;
        } else {
          socket[i]->schedule2.act=false;
        }
        save = true;
      }
      if (server.hasArg(tm11)) {          // First Schedule Start time is updated
        socket[i]->schedule1.on = strToTime(server.arg(tm11));
        save = true;
      }
      if (server.hasArg(tm12)) {          // First Schedule Stop time is updated
        socket[i]->schedule1.off = strToTime(server.arg(tm12));
        save = true;
      }
      if (server.hasArg(tm13)) {          // Second Schedule Start time is updated
        socket[i]->schedule2.on = strToTime(server.arg(tm13));
        save = true;
      }
      if (server.hasArg(tm14)) {          // Second Schedule Stop time is updated
        socket[i]->schedule2.off = strToTime(server.arg(tm14));
        save = true;
      }   
    }
    
  // Check if Feed Schedule changes
    sched1 = F("TCB16");
    sched2 = F("TCB17");
    tm11 =   F("TIM32");
    tm12 =   F("TIM33");
    tm13 =   F("TIM34");
    tm14 =   F("TIM35");
    if (server.hasArg(sched1)) {      // First Schedule Switch is updated
      //Serial.println(server.arg(sched1));
      if (server.arg(sched1) == "1") {  // On
        feedSchedule.schedule1.act=true;
      } else {                          // Off
        feedSchedule.schedule1.act=false;
      }
      save = true;
    }
    if (server.hasArg(sched2)) {    // Second Schedule Switch is updated
      if (server.arg(sched2) == "1") {  // On
        feedSchedule.schedule2.act=true;
      } else {                          // Off
        feedSchedule.schedule2.act=false;
      }
      save = true;
    }
    if (server.hasArg(tm11)) {      // Set First Schedule Start time
      feedSchedule.schedule1.on = strToTime(server.arg(tm11));
      save = true;
    }
    if (server.hasArg(tm12)) {      // Set First Schedule Stop time
      feedSchedule.schedule1.off = strToTime(server.arg(tm12));
      save = true;
    }
    if (server.hasArg(tm13)) {      // Set Second Schedule Start time
      feedSchedule.schedule2.on = strToTime(server.arg(tm13));
      save = true;
    }
    if (server.hasArg(tm14)) {    // Set Second Schedule Stop time
      feedSchedule.schedule2.off = strToTime(server.arg(tm14));
      save = true;
    }   

  // Check if Socket Schedule Feed Override mode is changed
  // For first Socket url argument will be ?C13=1
  // For second ?C14=1 etc
    #define SOCKET_FEED_BASE 13     
    for (i = 0; i < SOCKET_COUNT; i++) {
      cArg = (String(F("C")) + String(SOCKET_FEED_BASE + i));
      if (server.hasArg(cArg)) {
        v = server.arg(cArg);
        if (v == "1") {         // Override On
          socket[i]->feedOverride = SON;
        } else if (v == "0") {  // Override Off
          socket[i]->feedOverride = SOFF;
        } else {                // else Override set to NA
          socket[i]->feedOverride = SNA;
        }
      }   
    }
    
  // Check if got assign Socket to Group command
  // Assign First Socket to first group ?SOCG0=0
  // Assign Third Socket to 4-th group ?SOCG2=3
    for (i = 0; i < SOCKET_COUNT; i++) {
      cArg = F("SOCG");
      cArg += String(i);
      if (server.hasArg(cArg)) {
        v = server.arg(cArg);
        if (v == "11") {      // Group1
          socket[i]->setGroup(group[0]);
        } else if (v == "12") { // Group2
          socket[i]->setGroup(group[1]);
        } else if (v == "13") { // Group3
          socket[i]->setGroup(group[2]);
        } else if (v == "14") { // Group4
          socket[i]->setGroup(group[3]);
        } else {  // otherway set No Group
          socket[i]->setGroup();
        }
        save = true;
      }
    }
    
  // Check if got assign Socket Name
  // Assign First Socket name ?N0=First
  // Assign Third Socket name ?N2=Third
    for (i = 0; i < SOCKET_COUNT; i++) {
      nArg = F("N");
      nArg +=String(i);
      if (server.hasArg(nArg)) {
        v = server.arg(nArg);
        v.replace("<",  "&lt;");
        v.replace(">",  "&gt");
        v.replace("\"", "&quot;");
        socket[i]->name = v;
        save = true;
      }
    }
    
  // Check if Socket override mode or time is changed
  // Second Socket override to off ?C6=0 
  // New Mode will be saved to temprary variable until get actual period of time to override duration
    #define SOCKET_OVERRIDE_BASE 5
    for (i = 0; i < SOCKET_COUNT; i++) {
      cArg = (String(F("C")) + String(SOCKET_OVERRIDE_BASE + i));
      tArg = (String(F("CD")) + String(SOCKET_OVERRIDE_BASE + i));
      if (server.hasArg(cArg)) {
        v = server.arg(cArg);
        if (v == "1") {
          socket[i]->modeWaiting = SON;
        } else if (v == "0") {
          socket[i]->modeWaiting = SOFF;
        } else {
          socket[i]->modeWaiting = SNA;
        }
      }
      if (server.hasArg(tArg)) {  // If override time in minutes is passed ?CD6=15
        socket[i]->stop();        // Stop previous started Override if any
        socket[i]->start(socket[i]->modeWaiting, (int)(server.arg(tArg).toFloat()*60));
      }    
    }
    
  // Check if Group override mode or time is changed
  // First Group to Off ?C1=0
    #define GROUP_BASE 1
    #define GROUP_OVERRIDE 1
    for (i = 0; i < GROUP_COUNT; i++) {
      cArg = (String(F("C")) + String(GROUP_BASE + i));
      tArg = (String(F("CD")) + String(GROUP_OVERRIDE + i));
      if (server.hasArg(cArg)) {
        v = server.arg(cArg);
        if (v == "1") {
          group[i]->modeWaiting = SON;
        } else if (v == "0") {
          group[i]->modeWaiting = SOFF;
        } else {
          group[i]->modeWaiting = SNA;
        }
      }
      if (server.hasArg(tArg)) {
        group[i]->stop();     // Stop existing override if any
        group[i]->start(group[i]->modeWaiting, (int)(server.arg(tArg).toFloat()*60));
      }
    }
    
  // Feed override ?C0=1
    cArg = F("C0");
    tArg = F("CD0");
    if (server.hasArg(cArg)) {
      v = server.arg(cArg);
      if (v == "1") {
        feed->modeWaiting = SON;
      } else if (v == "0") {
        feed->modeWaiting = SOFF;
      } else {
        feed->modeWaiting = SNA;
      }
    }
    if (server.hasArg(tArg)) {
      feed->stop();
      feed->start(feed->modeWaiting, (int)(server.arg(tArg).toFloat()*60));
    }

  // Check if Wave mode is changed
  // ?SOCG8=121
    cArg = F("SOCG8");
    if (server.hasArg(cArg)) {
      v = server.arg(cArg);
      //if (pump != v && taskExists(wave.overrideTask))
        setPump(v);
        wave.on(wave.period);
        save = true;
    }
    cArg = F("W");
    if (server.hasArg(cArg)) {
      time_t t = strToTime24(server.arg(cArg));
      //if (wave.period != t && taskExists(wave.overrideTask)) {
        wave.on(t);
        save = true;
      //}
    }
    cArg = F("SEQ");
    if (server.hasArg(cArg)) {
      sequence = server.arg(cArg).toInt();
    }
  }
  float p1 = 0;
  float p2 = 0;
  float p3 = 0;
  if (history[l][0].Irms > I_NOISE_FLOOR && history[l][0].realPower > 0) p1 = history[l][0].realPower;
  if (history[l][1].Irms > I_NOISE_FLOOR && history[l][1].realPower > 0) p2 = history[l][1].realPower;
  if (history[l][2].Irms > I_NOISE_FLOOR && history[l][2].realPower > 0) p3 = history[l][2].realPower;
  sprintf_P(p, PSTR("<?xml version = \"1.0\" ?>\n<state>\n<analog>%d.%02d</analog><analog>%d.%02d</analog><analog>%d.%02d</analog>\n"),
              abs((int)p1), abs((int)(p1*100)%100),
              abs((int)p2), abs((int)(p2*100)%100),
              abs((int)p3), abs((int)(p3*100)%100));
  p += strlen(p);
  //Global feed mode
  sprintf_P(p, PSTR("<Switch>%s</Switch><Override>%lu</Override><Waiting>%s</Waiting>\n"),
              (feed->mode==SON)?"on":(feed->mode==SOFF)?"off":"default",
              taskRemainder(feedTask)/1000,
              (feed->modeWaiting==SON)?"on":(feed->modeWaiting==SOFF)?"off":"default"
              );
  p += strlen(p);
  for (i = 0; i < GROUP_COUNT; i++) {
    sprintf_P(p, PSTR("<Switch>%s</Switch><Override>%lu</Override><Waiting>%s</Waiting>\n"),
              (group[i]->mode==SON)?"on":(group[i]->mode==SOFF)?"off":"default",
              taskRemainder(groupOverride[i])/1000,
              (group[i]->modeWaiting==SON)?"on":(group[i]->modeWaiting==SOFF)?"off":"default"
              );
    p += strlen(p);
  }
  // Socket switch state
  for (i = 0; i < SOCKET_COUNT; i++) {
    sprintf_P(p, PSTR("<Manual>%s</Manual><Socket>%s</Socket><Enabled>%s</Enabled><SState>%s</SState>\n"), (socket[i]->manual == SON)?"checked":"unckecked", swState(socket[i]->mode).c_str(), socket[i]->enabled?"1":"0", socket[i]->actualState==SON?"1":"0");
    p += strlen(p);
  }
  // Socket override timers state and group
  for (i = 0; i < SOCKET_COUNT; i++) {
    uint8_t gr = 0;
    for (uint8_t j = 0; j < GROUP_COUNT; j++) {
      if (socket[i]->group == group[j]) {
        gr = j + GROUP_HTML_BASE;
        continue; 
      }
    }
    sprintf_P(p, PSTR("<TimerActive>%s</TimerActive><TimerActive>%s</TimerActive><TimerCheckbox>%s</TimerCheckbox>\n<TimerCheckbox>%s</TimerCheckbox>\n<TimerValue>%s</TimerValue>\n<TimerValue>%s</TimerValue>\n<TimerValue>%s</TimerValue>\n<TimerValue>%s</TimerValue><Duration>%s</Duration>\n<Group>%d</Group>\n<Override>%lu</Override><name>%s</name>"),
              socket[i]->schedule1.active(getTime())?"1":"0",
              socket[i]->schedule2.active(getTime())?"1":"0",
              socket[i]->schedule1.active()?"checked":"unckecked",
              socket[i]->schedule2.active()?"checked":"unckecked",
              timeToStr(socket[i]->schedule1.on).c_str(),
              timeToStr(socket[i]->schedule1.off).c_str(),
              timeToStr(socket[i]->schedule2.on).c_str(),
              timeToStr(socket[i]->schedule2.off).c_str(),
              timeToStr24(socket[i]->duration()/60).c_str(),
              gr,
              taskRemainder(socketTasks[i]) / 1000,
              socket[i]->name.c_str()
              );
    p += strlen(p);
  }
  for (i = 0; i < SOCKET_COUNT; i++) {
    sprintf_P(p, PSTR("<Switch>%s</Switch><Override>%lu</Override><Waiting>%s</Waiting>\n"),
              (socket[i]->mode==SON)?"on":(socket[i]->mode==SOFF)?"off":"default",
              taskRemainder(socketTasks[i])/1000,
              (socket[i]->modeWaiting==SON)?"on":(socket[i]->modeWaiting==SOFF)?"off":"default"
              );
    p += strlen(p);
  }
  for (i = 0; i < SOCKET_COUNT; i++) {
    sprintf_P(p, PSTR("<Switch>%s</Switch>"),
              (socket[i]->feedOverride==SON)?"on":(socket[i]->feedOverride==SOFF)?"off":"default"
              );
    p += strlen(p);
  }
  sprintf_P(p, PSTR("<TimerActive>%s</TimerActive><TimerActive>%s</TimerActive><TimerCheckbox>%s</TimerCheckbox>\n<TimerCheckbox>%s</TimerCheckbox>\n<TimerValue>%s</TimerValue>\n<TimerValue>%s</TimerValue>\n<TimerValue>%s</TimerValue>\n<TimerValue>%s</TimerValue><Duration>%s</Duration>\n"),
              feedSchedule.schedule1.active(getTime())?"1":"0",
              feedSchedule.schedule2.active(getTime())?"1":"0",
              feedSchedule.schedule1.active()?"checked":"unckecked",
              feedSchedule.schedule2.active()?"checked":"unckecked",
              timeToStr(feedSchedule.schedule1.on).c_str(),
              timeToStr(feedSchedule.schedule1.off).c_str(),
              timeToStr(feedSchedule.schedule2.on).c_str(),
              timeToStr(feedSchedule.schedule2.off).c_str(),
              timeToStr24(feedSchedule.duration()/60).c_str()
              );
  p += strlen(p);
  sprintf_P(p, PSTR("<Pump>%s</Pump><Wave>%s</Wave><time>%s</time><sequence>%lu</sequence>\n"),
              pump.c_str(), timeToStr24(wave.period).c_str(), timeToStr(getTime()).c_str(), sequence);
  p += strlen(p);
  /*
  int16_t* pV = (int16_t*)data;
  int16_t* pI = (int16_t*)data + (SAVE_SAMPLES * sizeof(int16_t));
  for (i = 0; i < SAVE_SAMPLES; i++) {
    sprintf_P(p, PSTR("<v>%d</v><i>%d</i>\n"),
              pV[i], pI[i]);
    p += strlen(p);
  }
  p += strlen(p);
  */
  sprintf_P(p, PSTR("\n</state>"));
  if (save) {           // If save flag set true queue save state
    taskDel(saveState); // Remove previous save request if any
    taskAddWithDelay(saveState, SAVE_DELAY);  // save in SAVE_DELAY mS
  }
  WDEBUG("Mem: %d, DATA: %d\n", ESP.getFreeHeap(), strlen(xmlBuffer));
  server.sendHeader(F("Connection"), F("close"));                       // Headers again free connection and
  server.sendHeader(F("Cache-Control"), F("no-store, must-revalidate"));// Don't chaching
  server.sendHeader(F("Access-Control-Allow-Origin"), F("*"));          // Helpful when page contains JavaScript code that performing outgoung requests
  server.send(200, "text/xml", xmlBuffer);                // Send string as XML document to cliend.
                                                          // 200 - means Success html result code
}

// callback function that is called by Web server if no sutable callback function fot URL found
void indexFile() {
    server.sendHeader("Connection", "close");                       // Headers again free connection and
    server.sendHeader("Cache-Control", "no-store, must-revalidate");// Don't chaching
    server.sendHeader("Access-Control-Allow-Origin", "*");          // Helpful when page contains JavaScript code that performing outgoung requests
    File file = SPIFFS.open(INDEX, "r");                            // Open default index file readonly
    size_t sent = server.streamFile(file, "text/html");             // Send file to clent as HTML document
    file.close();                                                   // Close file
}
// Building Web-page to upload files, delete files and list files located on ESP flash file system
void listFile() {
  // Authentification
#ifdef UPLOADPASS
  if(!server.authenticate(UPLOADUSER, UPLOADPASS)) {
    return server.requestAuthentication();
  }
#endif
  byte mac[6];
  WiFi.macAddress(mac);
  String ip = WiFi.localIP().toString();
  char* p = data;
  sprintf_P(p, PSTR("<html><head><meta charset='utf-8'>\
  <title>WiFiSocket - Maintains</title>\
\
 <style>\
.well{background-image:-webkit-linear-gradient(top,#e8e8e8 0,#f5f5f5 100%%);\
background-image:-o-linear-gradient(top,#e8e8e8 0,#f5f5f5 100%%);\
background-image:-webkit-gradient(linear,left top,left bottom,from(#e8e8e8),to(#f5f5f5));\
background-image:linear-gradient(to bottom,#e8e8e8 0,#f5f5f5 100%%);\
filter:progid:DXImageTransform.Microsoft.gradient(startColorstr='#ffe8e8e8', endColorstr='#fff5f5f5', GradientType=0);\
background-repeat:repeat-x;\
border-color:#dcdcdc;\
-webkit-box-shadow:inset 0 1px 3px rgba(0,0,0,.05),0 1px 0 rgba(255,255,255,.1);\
box-shadow:inset 0 1px 3px rgba(0,0,0,.05),0 1px 0 rgba(255,255,255,.1)}\
.well{min-height:20px;padding:19px;margin-bottom:20px;background-color:#f5f5f5;border:1px solid #e3e3e3;border-radius:4px;-webkit-box-shadow:inset 0 1px 1px rgba(0,0,0,.05);box-shadow:inset 0 1px 1px rgba(0,0,0,.05)}\
.well blockquote{border-color:#ddd;border-color:rgba(0,0,0,.15)}\
.container{padding-right:15px;padding-left:15px;margin-right:auto;margin-left:auto}\
.col-md-4{display: table-cell;min-height:1px;padding-right:15px;padding-left:15px;width:33%%;}\
.row{display: table;width: 100%%;margin-right:-15px;margin-left:-15px}\
body{font-family:\"Helvetica Neue\",Helvetica,Arial,sans-serif;font-size:14px;line-height:1.42857143;color:#333;background-color:#fff}\
h4{font-size:18px;text-align:left;}\
h1{font-size:10px;text-align:right;}\
#progress {color: fff;text-align: center;visibility: hidden;z-index: 10;display: block;border: 1px inset #446;border-radius: 5px;position: fixed;bottom: 0;width: 80%%;left: 50%%;transform: translate(-50%%, -50%%);margin: 0 auto;background: linear-gradient(to right, #0c0 0%%, #000 0%%);}\
#progress .success {background: #0c0 none 0 0 no-repeat;}\
#progress .failed {color: #fff;font-wheight: bold;background: #c00 none 0 0 no-repeat;}\
#advBlock {visibility: hidden;}\
 </style>\
<script>\
function disableForm(form) {\
  var elements = form.elements;\
  for (var i = 0, len = elements.length; i < len; ++i) {\
  elements[i].disabled = true;\
  }\
}\
function uploadFile(file) {\
var xhr = new XMLHttpRequest();\
this.xhr = xhr;\
var reader = new FileReader();\
if (xhr.upload && file.size) {\
document.getElementsByName('form').forEach(disableForm);\
var progress = document.getElementById('progress');\
progress.innerHTML = 'Uploading firmware...';\
progress.style.visibility='visible';\
xhr.upload.addEventListener('progress', function(e) {\
var pc = parseInt(e.loaded / e.total * 100);\
document.getElementById('progress').style.background = 'linear-gradient(to right, #0c0 ' + pc + '%%, ' + ' #000 ' + pc + '%%)';\
}, false);\
xhr.onreadystatechange = function(e) {\
if (xhr.readyState == 4) {\
if (xhr.status != 200) {\
document.getElementById('progress').innerHTML='Firmware uploading failed';\
document.getElementById('progress').style.background = '#c00';\
setTimeout('location.reload();', 10000);\
} else {\
document.getElementById('progress').innerHTML='Rebooting...';\
setTimeout('location.reload();', 5000);\
}\
}\
};\
var formData = new FormData();\
formData.append('userfile', file);\
xhr.open('POST', '/update', true);\
xhr.send(formData);\
}\
}\
function advanced() {\
  var adv = document.getElementById('advBlock');\
  var advButton = document.getElementById('advShow');\
  adv.style.visibility='visible';\
  advButton.style.visibility='hidden';\
}\
var cTime= new Date(%d * 1000);\
function countTime() {\
 var hours = cTime.getUTCHours();\
 var minutes = cTime.getUTCMinutes();\
 var ampm = hours >= 12?'PM':'AM';\
 hours = hours %% 12;\
 hours = hours ? hours : 12;\
 minutes = minutes < 10 ? '0'+minutes : minutes;\
 document.getElementById('advTime').innerHTML = hours + ':' + minutes + '' + ampm;\
 cTime = new Date(cTime.getTime() + 1000);\
 setTimeout(countTime, 1000);\
 }\
</script>\
  </head>\
  <body onLoad='countTime();'>\
\
 <div class='row'><div class='col-md-4'><h4>Wi-Fi Socket Control - Settings</h4></div><div class='col-md-4'><h4 id='advTime'>00:00</h4></div><div class='col-md-4'><h1><a id='advShow' href='javascript:advanced()'>Show Advanced settings</a></h1></div>\
 </div>\
 <div class='container'><div class='well'>\
 <b>Network settings</b>\
 <hr>\
 <form name='form' method='POST' action='/net' enctype='multipart/form-data'>\
  <table>\
  <tr><td>IP</td><td>%s</td></tr>\
  <tr><td>Unit Name</td><td><input name=\"name\" value=\"%s\"></td></tr>\
  <tr><td>TimeZone</td><td><select name=\"tz\">\
  <script>tz=%s;tzs =[\
        {str: \"MIT  Midway Islands Time (GMT-11:00)\", offset: -11},\
        {str: \"HST Hawaii Standard Time  (GMT-10:00)\", offset: -10},\
        {str: \"AST Alaska Standard Time  (GMT-9:00)\", offset: -9},\
        {str: \"PST Pacific Standard Time (GMT-8:00)\", offset: -8},\
        {str: \"PNT Phoenix Standard Time (GMT-7:00)\", offset: -7},\
        {str: \"MST Mountain Standard Time  (GMT-7:00)\", offset: -7},\
        {str: \"CST Central Standard Time (GMT-6:00)\", offset: -6},\
        {str: \"EST Eastern Standard Time (GMT-5:00)\", offset: -5},\
        {str: \"IET Indiana Eastern Standard Time (GMT-5:00)\", offset: -5},\
        {str: \"PRT Puerto Rico and US Virgin Islands Time  (GMT-4:00)\", offset: -4},\
        {str: \"AGT Argentina Standard Time (GMT-3:00)\", offset: -3},\
        {str: \"BET Brazil Eastern Time (GMT-3:00)\", offset: -3},\
        {str: \"CAT Central African Time  (GMT-1:00)\", offset: -1},\
        {str: \"UTC  Universal Coordinated Time  (GMT)\", offset: 0},\
        {str: \"ECT European Central Time (GMT+1:00)\", offset: 1},\
        {str: \"EET Eastern European Time (GMT+2:00)\", offset: 2},\
        {str: \"ART (Arabic) Egypt Standard Time  (GMT+2:00)\", offset: 2},\
        {str: \"EAT Eastern African Time  (GMT+3:00)\", offset: 3},\
        {str: \"NET Near East Time  (GMT+4:00)\", offset: 4},\
        {str: \"PLT Pakistan Lahore Time  (GMT+5:00)\", offset: 5},\
        {str: \"BST Bangladesh Standard Time  (GMT+6:00)\", offset: 6},\
        {str: \"VST Vietnam Standard Time (GMT+7:00)\", offset: 7},\
        {str: \"CTT China Taiwan Time (GMT+8:00)\", offset: 8},\
        {str: \"JST Japan Standard Time GMT+9:00\", offset: 9},\
        {str: \"AET Australia Eastern Time  (GMT+10:00)\", offset: 10},\
        {str: \"SST Solomon Standard Time (GMT+11:00)\", offset: 11},\
        {str: \"NST New Zealand Standard Time (GMT+12:00)\", offset: 12}\
        ];\
  for (t = 0; t < tzs.length; t++) {\
    document.write(\"<option value='\");\
    document.write(tzs[t].offset);\
    document.write(\"'\");\
    if (tzs[t].offset == tz)\
    document.write('selected');\
    document.write(\">\");\
    document.write(tzs[t].str);\
    document.write(\"</option>\");\
  }\
  </script>\
  </select>&nbsp;<font size=-2>(Become effective on Socket restart)</font></td></tr>\
  <tr><td>Setup AP SSID</td><td>%s%02X%02X</td></tr>\
  </table>\
  <input type='submit' value='Apply'>\
  </table>\
 </form>\
 </div></div>\
\
<div class='container'><div class='well'>\
 <b>Firmware update</b>\
 <hr>\
 Current version: %s\
  <form name='form' method='POST' action='/update' enctype='multipart/form-data'><input id='update' type='file' name='update' accept='.tar'><input type='button' value='Update' onClick='uploadFile(document.getElementById(\"update\").files[0]);'></form>\
</div></div>\
\
<div class='container'><div class='well'>\
 <b>Resets</b>\
 <hr>\
  <form name='form' method='POST' action='/list' enctype='multipart/form-data'>\
  <input type='button' value='Reboot' onClick='if(confirm(\"Reboot device?\")) window.location=\"/reboot\";return true;'><br>\
  <input type='button' value='Reset to defaults' onClick='if(confirm(\"Reset settings to defaults?\")) window.location=\"/default\";return true;'></form>\
</div></div>\
<div id='advBlock'>\
<div class='container'><div class='well'>\
 <b>Local file system</b>\
 <hr>\
  <form method='POST' action='/edit' enctype='multipart/form-data'>\
  Upload file to local filesystem:<br>\
   <input type='file' name='update'>\
   <input type='submit' value='Upload file'>\
  </form>"), getTime(), ip.c_str(), sysName.c_str(), tz.c_str(), WIFI_SETUP_AP, mac[4], mac[5], VERSION);
  p += strlen(p);
  String path = server.hasArg("dir")?server.arg("dir"):"/";
  Dir dir = SPIFFS.openDir(path);
  while(dir.next()){
    File entry = dir.openFile("r");
    sprintf_P(p, PSTR("<br><a href='%s'>%s</a>&nbsp<a href='/delete?file=%s'><font color=red>delete</font></a><br>"), entry.name(), entry.name(), entry.name());
    p += strlen(p);
    entry.close();
  }
  sprintf_P(p, PSTR("</div></div><div class='container'><div class='well'>\
 <b>NTP Settings</b>\
 <hr>\
 <table>\
  <tr><td>NTP Server 1</td><td><input name=\"ntp1\" value=\"%s\"></td></tr>\
  <tr><td>NTP Server 2</td><td><input name=\"ntp2\" value=\"%s\"></td></tr>\
  <tr><td>NTP Server 3</td><td><input name=\"ntp3\" value=\"%s\"></td></tr>\
</table>\
</div></div></div>\
<div id='progress'></div>\
</body><html>"), ntp1.c_str(), ntp2.c_str(), ntp3.c_str());

  server.sendHeader("Connection", "close");
  server.sendHeader("Cache-Control", "no-store, must-revalidate");
  server.sendHeader("Access-Control-Allow-Origin", "*");
  WDEBUG("Pre Mem: %d, DATA: %d\n", ESP.getFreeHeap(), strlen(data));
  server.send(200, "text/html", data);
  WDEBUG("Post Mem: %d, DATA: %d\n", ESP.getFreeHeap(), strlen(data));
}
// File upload. Called on upload finished
void handleFile() {
#ifdef UPLOADPASS
  if(!server.authenticate(UPLOADUSER, UPLOADPASS)) {
    return server.requestAuthentication();
  }
#endif
  server.sendHeader("Connection", "close");
  server.sendHeader("Cache-Control", "no-store, must-revalidate");
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.sendHeader("Refresh", "0; url=/list");
  server.send_P(200, "text/plain", PSTR("OK"));  
}
// File upload. Called on data received
File fsUploadFile;
void handleFileUpload(){
  //Serial.println("UPLOAD");
#ifdef UPLOADPASS
  if(!server.authenticate(UPLOADUSER, UPLOADPASS)) {
    return server.requestAuthentication();
  }
#endif
  if(server.uri() != "/edit") return;
  BUSY
  HTTPUpload& upload = server.upload();
  if(upload.status == UPLOAD_FILE_START){
    String filename = upload.filename;
    if(!filename.startsWith("/")) filename = "/"+filename;
    fsUploadFile = SPIFFS.open(filename, "w");
    filename = String();
  } else if(upload.status == UPLOAD_FILE_WRITE){
    if(fsUploadFile)
      fsUploadFile.write(upload.buf, upload.currentSize);
  } else if(upload.status == UPLOAD_FILE_END){
    if(fsUploadFile)
      fsUploadFile.close();
  }
  IDLE
}
// Read file routine. Used internaly
bool fileRead(String path){
  if(path.endsWith("/")) path += INDEX;
  String contentType = StaticRequestHandler::getContentType(path);
  String pathWithGz = path + ".gz";
  if(SPIFFS.exists(pathWithGz) || SPIFFS.exists(path)){
    if(SPIFFS.exists(pathWithGz))
      path += ".gz";
    server.sendHeader("Connection", "close");
    server.sendHeader("Cache-Control", "no-store, must-revalidate");
    server.sendHeader("Access-Control-Allow-Origin", "*");
    File file = SPIFFS.open(path, "r");
    size_t sent = server.streamFile(file, contentType);
    file.close();
    return true;
  }
  return false;
}
void anyFile() {
  BUSY
  if(!fileRead(server.uri()))
    server.send_P(404, "text/plain", PSTR("FileNotFound"));
  IDLE
}
// Delete file callback
void handleDelete() {
#ifdef UPLOADPASS
  if(!server.authenticate(UPLOADUSER, UPLOADPASS)) {
    return server.requestAuthentication();
  }
#endif
  server.sendHeader("Connection", "close");
  server.sendHeader("Cache-Control", "no-store, must-revalidate");
  server.sendHeader("Refresh", "0; url=/list");
  String path;
  if(server.args() != 0) {
    path = server.arg(0);
    if(path != "/" && SPIFFS.exists(path)) {
      if (SPIFFS.remove(path)) {
        server.send(200, "text/plain", "OK");
        IDLE
        return;
      } else {
        server.send_P(404, "text/plain", PSTR("FileNotFound"));
        IDLE
        return;
      }
    }
  }
  server.send(500, "text/plain", "ERROR");
  IDLE
}

void handleReboot() {
#ifdef UPLOADPASS
  if(!server.authenticate(UPLOADUSER, UPLOADPASS)) {
    return server.requestAuthentication();
  }
#endif
  saveState();
  server.sendHeader("Connection", "close");
  server.sendHeader("Cache-Control", "no-store, must-revalidate");
  server.sendHeader("Refresh", "7; url=/");
  taskAddWithDelay(restartESP, 1000);
  server.send_P(200, "text/plain", PSTR("Rebooting..."));
}

void handleResetToDefaults() {
#ifdef UPLOADPASS
  if(!server.authenticate(UPLOADUSER, UPLOADPASS)) {
    return server.requestAuthentication();
  }
#endif
  server.sendHeader("Connection", "close");
  server.sendHeader("Cache-Control", "no-store, must-revalidate");
  server.sendHeader("Refresh", "7; url=http://socket/");
  taskAddWithDelay(restartESP, 1000);
  SPIFFS.remove(STATE);
  SPIFFS.remove(CFG);
  server.send_P(200, "text/plain", PSTR("Settings was reset. Rebooting..."));
}

void handleNetwork() {
#ifdef UPLOADPASS
  if(!server.authenticate(UPLOADUSER, UPLOADPASS)) {
    return server.requestAuthentication();
  }
#endif
  server.sendHeader("Connection", "close");
  server.sendHeader("Cache-Control", "no-store, must-revalidate");
  server.sendHeader("Refresh", "0; url=/list");
  if(server.hasArg("ntp1")) {
    ntp1 = server.arg("ntp1");
  }
  if(server.hasArg("ntp2")) {
    ntp2 = server.arg("ntp2");
  }
  if(server.hasArg("ntp3")) {
    ntp3 = server.arg("ntp3");
  }
  if(server.hasArg("tz")) {
    tz = server.arg("tz");
  }
  if(server.hasArg("name")) {
    sysName = server.arg("name");
  }
  saveConfig();
  server.send(200, "text/plain", "OK");
}

#define DEFAULT_OVERRIDE 10
void handleOverride() {
  server.sendHeader("Connection", "close");
  server.sendHeader("Cache-Control", "no-store, must-revalidate");
  String md;
  time_t tm = DEFAULT_OVERRIDE;
  if(server.hasArg("time")) {
    tm = server.arg("time").toInt();
  }
  if(server.hasArg("mode")) {
    if (server.arg("mode") == "ON") {
      socket[2]->start(SON, tm * 1000L);
      return;
    } else if (server.arg("mode") == "OFF") {
      socket[2]->start(SOFF, tm * 1000L);
      return;
    } else {
      ;
    }
    server.send(200, "text/plain", "OK");
  }
  server.send(500, "text/plain", "ERROR");
  IDLE
}

// raw data for debug
void handleSamples() {
  String csv;
  uint16_t i;
  int16_t* V1 = (int16_t*)data;
  int16_t* I1 = (int16_t*)data +  512;
  int16_t* V2 = (int16_t*)data + 1024;
  int16_t* I2 = (int16_t*)data + 1536;
  int16_t* V3 = (int16_t*)data + 2048;
  int16_t* I3 = (int16_t*)data + 2560;
  int16_t* VCC = (int16_t*)data + 3072;
  for (i = 0; i < 256; i++) {
    V1[i] = mcp3221_read(MCP_V);
    I1[i] = mcp3221_read(MCP_0);
  }
  VCC[0] = ESP.getVcc();
  for (i = 0; i < 256; i++) {
    V2[i] = mcp3221_read(MCP_V);
    I2[i] = mcp3221_read(MCP_1);
  }
  VCC[1] = ESP.getVcc();
  for (i = 0; i < 256; i++) {
    V3[i] = mcp3221_read(MCP_V);
    I3[i] = mcp3221_read(MCP_3);
  }
  VCC[2] = ESP.getVcc();
  server.sendHeader("Connection", "close");
  server.sendHeader("Cache-Control", "no-store, must-revalidate");
  for (i = 0; i < 256; i++) {
    csv += String(V1[i]) + ", ";
    csv += String(I1[i]) + "\n";
  }
  csv += "\n";
  for (i = 0; i < 256; i++) {
    csv += String(V2[i]) + ", ";
    csv += String(I2[i]) + "\n";
  }
  csv += "\n";
  for (i = 0; i < 256; i++) {
    csv += String(V3[i]) + ", ";
    csv += String(I3[i]) + "\n";
  }
  csv += "\n";

  for (i = 0; i < 3; i++) {
    csv += "Vcc=";
    csv += String(history[l][i].Vcc);
    csv += "\n";
    csv += "realPower=";
    csv += String(history[l][i].realPower);
    csv += "; apparentPower=";
    csv += String(history[l][i].apparentPower);
    csv += "; powerFactor=";
    csv += String(history[l][i].powerFactor);
    csv += "; Vrms=";
    csv += String(history[l][i].Vrms);
    csv += "; Irms=";
    csv += String(history[l][i].Irms);
    csv += "\n";
  }
  server.send(200, "text/csv", csv);
  IDLE
}

void handleHistory() {
  String csv;
  
  csv += F("Vcc_1,realPower_0,apparentPower_0,powerFactor_0,Vrms_0,Irms_0,Vtune_0,Itune_0,");
  csv += F("Vcc_1,realPower_1,apparentPower_1,powerFactor_1,Vrms_1,Irms_1,Vtune_1,Itune_1,");
  csv += F("Vcc_1,realPower_2,apparentPower_2,powerFactor_2,Vrms_2,Irms_2,Vtune_2,Itune_2\n");
  for (uint16_t i = 0; i < HISTORY; i++) {
    if (i == l) {
      csv += "0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0\n";
    }
    for (uint8_t j = 0; j < MCP_COUNT; j++) {
      String csv1 = String(history[i][j].Vcc);
      csv1 += ",";
      csv1 += String(history[i][j].realPower, 5);
      csv1 += ",";
      csv1 += String(history[i][j].apparentPower, 5);
      csv1 += ",";
      csv1 += String(history[i][j].powerFactor, 5);
      csv1 += ",";
      csv1 += String(history[i][j].Vrms, 5);
      csv1 += ",";
      csv1 += String(history[i][j].Irms,5);
      csv1 += ",";
      csv1 += String(history[i][j].Vtune,5);
      csv1 += ",";
      csv1 += String(history[i][j].Itune,5);
      csv1 += ",";
      csv += csv1;
    }
    csv += "\n";
  }
  server.send(200, "text/csv", csv);
  IDLE
}

uint32_t webHandle() {
  server.handleClient();
  return 100;
}

uint32_t initWeb() {
  //Serial.println("Init WebServer");
    server.on("/ajax_inputs", HTTP_GET, ajaxInputs);  // call function ajaxInputs() if Web Server gets request http://192.168.1.20/ajax_inputs?LED1=0...
    // You can add multiple server.on(url...) to handle different url by specific routines
    //server.on(INDEX, HTTP_GET, indexFile);                    // call function indexFile() on GET <INDEX>
    server.on("/list", HTTP_GET, listFile);                   // List/Upload/Delete page
    server.on("/delete", HTTP_GET, handleDelete);             // Delete File
    server.on("/edit", HTTP_POST, handleFile, handleFileUpload);    // Upload file
    server.onNotFound(anyFile);                               // call function anyFile() on any other requests
    //server.on("/socket", HTTP_GET, handleOverride);
    server.on("/net", HTTP_POST, handleNetwork);
    server.on("/reboot", HTTP_GET, handleReboot);
    server.on("/default", HTTP_GET, handleResetToDefaults);
    server.on("/samples.csv", HTTP_GET, handleSamples);
    server.on("/history.csv", HTTP_GET, handleHistory);
   #ifdef WFS_DEBUG
    server.on("/debug", HTTP_GET, handleDebug);
   #endif
    server.begin();                                           // start to listen for clients 
    taskAdd(webHandle);
    return RUN_DELETE;
}
