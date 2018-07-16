#pragma once
#include <detail/RequestHandlersImpl.h>

#define INDEX "index.html"
#define UPLOADUSER  "admin"
#define UPLOADPASS "password"
#undef UPLOADPASS
#define BUSY ;
#define IDLE ;
#define SAVE_DELAY 5000

ESP8266WebServer server(80);      // create a server at port 80
uint32_t sequence = 0;

void handleDebug() {
  char data[400];
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
  sprintf(data,\
          ("<?xml version = \"1.0\" encoding=\"UTF-8\" ?><private><heap>%d</heap><rssi>%d</rssi><uptime>%ld</uptime>\
          <rtcinit>%s</rtcinit><rtcpower>%s</rtcpower><rtc>%ld</rtc><ntp>%ld</ntp><sys>%ld</sys><s1>%s</s1><s2>%s</s2><s3>%s</s3><n1>%s</n1><n2>%s</n2><n3>%s</n3></private>"),\
          ESP.getFreeHeap(), WiFi.RSSI(), (uint32_t)millis()/1000,\
          (!status.rtcPresent)?"Failed":"Ok", rtc.lostPower()?"Failed":"Ok", now.unixtime(), time(NULL), getTime(),\
          ntp1.c_str(), ntp2.c_str(), ntp3.c_str(), (n1.toString()).c_str(), (n2.toString()).c_str(), (n3.toString()).c_str()\
          );
  server.sendHeader("Connection", "close");
  server.sendHeader("Cache-Control", "no-store, must-revalidate");
  server.send(200, "text/xml", data);  
}

uint32_t restartESP();
String swState(OverrideMode o) {
  return (o==SON)?"on":(o==SOFF)?"off":"default";
}
// callback function that is called by Web server in case if /ajax_input?...
void ajaxInputs() {
  char data[400];   // sprintf buffer
  uint8_t i;
  char  strTime[4][8];    // Schedule time strings buffer
  uint16_t  minutesFromMidnight;
  bool save = false;
  server.sendHeader("Connection", "close");                         // Headers to free connection ASAP and 
  server.sendHeader("Cache-Control", "no-store, must-revalidate");  // Don't cache response
  // Check if got Socket on/off switching or Schedule changes
  for (i = 0; i < SOCKET_COUNT; i++) {
    String soc = "SOC"+String(i);
    if (server.hasArg(soc)) {
      if (server.arg(soc) == "1") {
        //socket[i]->on();
        socket[i]->manual = SON;
      } else {
        socket[i]->manual = SOFF;
        //socket[i]->na();
      }
    }
    String sched1 = "TCB"+String(i*2);    // e.g. ?TCB2=1
    String sched2 = "TCB"+String(i*2+1);  // e.g. ?TCB3=1
    String tm11 =   "TIM"+String(i*4);    // e.g. ?TIM8=10:15AM
    String tm12 =   "TIM"+String(i*4+1);  // e.g. ?TIM9=11:00PM
    String tm13 =   "TIM"+String(i*4+2);  // e.g. ?TIM10=12:00PM
    String tm14 =   "TIM"+String(i*4+3);  // e.g. ?TIM11=21:45PM
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
  {
    String sched1 = "TCB16";
    String sched2 = "TCB17";
    String tm11 =   "TIM32";
    String tm12 =   "TIM33";
    String tm13 =   "TIM34";
    String tm14 =   "TIM35";
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
  }
  // Check if Socket Schedule Feed Override mode is changed
  // For first Socket url argument will be ?C13=1
  // For second ?C14=1 etc
  #define SOCKET_FEED_BASE 13     
  for (i = 0; i < SOCKET_COUNT; i++) {
    String cArg = "C" + String(SOCKET_FEED_BASE + i);
    if (server.hasArg(cArg)) {
      String v = server.arg(cArg);
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
    String cArg = "SOCG" + String(i);
    if (server.hasArg(cArg)) {
      String v = server.arg(cArg);
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
    String nArg = "N" + String(i);
    if (server.hasArg(nArg)) {
      String name = server.arg(nArg);
      name.replace("<",  "&lt;");
      name.replace(">",  "&gt");
      name.replace("\"", "&quot;");
      socket[i]->name = name;
      save = true;
    }
  }
  // Check if Socket override mode or time is changed
  // Second Socket override to off ?C6=0 
  // New Mode will be saved to temprary variable until get actual period of time to override duration
  #define SOCKET_OVERRIDE_BASE 5
  for (i = 0; i < SOCKET_COUNT; i++) {
    String cArg = "C" + String(SOCKET_OVERRIDE_BASE + i);
    String tArg = "CD" + String(SOCKET_OVERRIDE_BASE + i);
    if (server.hasArg(cArg)) {
      String v = server.arg(cArg);
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
    String cArg = "C" + String(GROUP_BASE + i);
    String tArg = "CD" + String(GROUP_OVERRIDE + i);
    if (server.hasArg(cArg)) {
      String v = server.arg(cArg);
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
  {
    String cArg = "C0";
    String tArg = "CD0";
    if (server.hasArg(cArg)) {
      String v = server.arg(cArg);
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
  }
  // Check if Wave mode is changed
  // ?SOCG8=121
  {
    String cArg = "SOCG8";
    if (server.hasArg(cArg)) {
      String v = server.arg(cArg);
      //if (pump != v && taskExists(wave.overrideTask))
        setPump(v);
        wave.on(wave.period);
        save = true;
    }
  }
  {
    String cArg = "W";
    if (server.hasArg(cArg)) {
      time_t t = strToTime24(server.arg(cArg));
      //if (wave.period != t && taskExists(wave.overrideTask)) {
        wave.on(t);
        save = true;
      //}
    }
  }
  {
    String cArg = "SEQ";
    if (server.hasArg(cArg)) {
      sequence = server.arg(cArg).toInt();
    }
  }
  // Assemble current state xml
  String res = "";
  String an = String(current()*110);  // Convert to Wattage
  sprintf_P(data, PSTR("<?xml version = \"1.0\" ?>\n<state>\n<analog>%s</analog>\n"), an.c_str());
  res += data;
  //Global feed mode
  sprintf_P(data, PSTR("<Switch>%s</Switch><Override>%lu</Override><Waiting>%s</Waiting>"),
              (feed->mode==SON)?"on":(feed->mode==SOFF)?"off":"default",
              taskRemainder(feedTask)/1000,
              (feed->modeWaiting==SON)?"on":(feed->modeWaiting==SOFF)?"off":"default"
              );
  res += data;
  res += "\n";
  for (i = 0; i < GROUP_COUNT; i++) {
    sprintf_P(data, PSTR("<Switch>%s</Switch><Override>%lu</Override><Waiting>%s</Waiting>"),
              (group[i]->mode==SON)?"on":(group[i]->mode==SOFF)?"off":"default",
              taskRemainder(groupOverride[i])/1000,
              (group[i]->modeWaiting==SON)?"on":(group[i]->modeWaiting==SOFF)?"off":"default"
              );
    res += data;
  }
  res += "\n";
  // Socket switch state
  for (i = 0; i < SOCKET_COUNT; i++) {
    sprintf_P(data, PSTR("<Manual>%s</Manual><Socket>%s</Socket><Enabled>%s</Enabled><SState>%s</SState>\n"), (socket[i]->manual == SON)?"checked":"unckecked", swState(socket[i]->mode).c_str(), socket[i]->enabled?"1":"0", socket[i]->actualState==SON?"1":"0");
    res += data;
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
    sprintf_P(data, PSTR("<TimerActive>%s</TimerActive><TimerActive>%s</TimerActive><TimerCheckbox>%s</TimerCheckbox>\n<TimerCheckbox>%s</TimerCheckbox>\n<TimerValue>%s</TimerValue>\n<TimerValue>%s</TimerValue>\n<TimerValue>%s</TimerValue>\n<TimerValue>%s</TimerValue><Duration>%s</Duration>\n<Group>%d</Group>\n<Override>%lu</Override><name>%s</name>"),
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
    res += data;
  }
  for (i = 0; i < SOCKET_COUNT; i++) {
    sprintf_P(data, PSTR("<Switch>%s</Switch><Override>%lu</Override><Waiting>%s</Waiting>"),
              (socket[i]->mode==SON)?"on":(socket[i]->mode==SOFF)?"off":"default",
              taskRemainder(socketTasks[i])/1000,
              (socket[i]->modeWaiting==SON)?"on":(socket[i]->modeWaiting==SOFF)?"off":"default"
              );
    res += data;
  }
  for (i = 0; i < SOCKET_COUNT; i++) {
    sprintf_P(data, PSTR("<Switch>%s</Switch>"),
              (socket[i]->feedOverride==SON)?"on":(socket[i]->feedOverride==SOFF)?"off":"default"
              );
    res += data;
  }
  sprintf_P(data, PSTR("<TimerActive>%s</TimerActive><TimerActive>%s</TimerActive><TimerCheckbox>%s</TimerCheckbox>\n<TimerCheckbox>%s</TimerCheckbox>\n<TimerValue>%s</TimerValue>\n<TimerValue>%s</TimerValue>\n<TimerValue>%s</TimerValue>\n<TimerValue>%s</TimerValue><Duration>%s</Duration>\n"),
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
  res += data;
  sprintf_P(data, PSTR("<Pump>%s</Pump><Wave>%s</Wave><time>%s</time><sequence>%lu</sequence>"),
              pump.c_str(), timeToStr24(wave.period).c_str(), timeToStr(getTime()).c_str(), sequence);
  res += data;
  res += "</state>";
  if (save) {           // If save flag set true queue save state
    taskDel(saveState); // Remove previous save request if any
    taskAddWithDelay(saveState, SAVE_DELAY);  // save in SAVE_DELAY mS
  }
  server.send(200, "text/xml", res);                      // Send string as XML document to cliend.
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
  String output = F("<html><head><meta charset='utf-8'>\
  <title>WiFiSocket - Maintains</title>\
\
 <style>\
.well{background-image:-webkit-linear-gradient(top,#e8e8e8 0,#f5f5f5 100%);\
background-image:-o-linear-gradient(top,#e8e8e8 0,#f5f5f5 100%);\
background-image:-webkit-gradient(linear,left top,left bottom,from(#e8e8e8),to(#f5f5f5));\
background-image:linear-gradient(to bottom,#e8e8e8 0,#f5f5f5 100%);\
filter:progid:DXImageTransform.Microsoft.gradient(startColorstr='#ffe8e8e8', endColorstr='#fff5f5f5', GradientType=0);\
background-repeat:repeat-x;\
border-color:#dcdcdc;\
-webkit-box-shadow:inset 0 1px 3px rgba(0,0,0,.05),0 1px 0 rgba(255,255,255,.1);\
box-shadow:inset 0 1px 3px rgba(0,0,0,.05),0 1px 0 rgba(255,255,255,.1)}\
.well{min-height:20px;padding:19px;margin-bottom:20px;background-color:#f5f5f5;border:1px solid #e3e3e3;border-radius:4px;-webkit-box-shadow:inset 0 1px 1px rgba(0,0,0,.05);box-shadow:inset 0 1px 1px rgba(0,0,0,.05)}\
.well blockquote{border-color:#ddd;border-color:rgba(0,0,0,.15)}\
.container{padding-right:15px;padding-left:15px;margin-right:auto;margin-left:auto}\
.col-xs-9{position:relative;min-height:1px;padding-right:15px;padding-left:15px}\
body{font-family:\"Helvetica Neue\",Helvetica,Arial,sans-serif;font-size:14px;line-height:1.42857143;color:#333;background-color:#fff}\
h4,h4{font-size:18px}\
#progress {color: fff;text-align: center;visibility: hidden;z-index: 10;display: block;border: 1px inset #446;border-radius: 5px;position: fixed;bottom: 0;width: 80%;left: 50%;transform: translate(-50%, -50%);margin: 0 auto;background: linear-gradient(to right, #0c0 0%, #000 0%);}\
#progress .success {background: #0c0 none 0 0 no-repeat;}\
#progress .failed {color: #fff;font-wheight: bold;background: #c00 none 0 0 no-repeat;}\
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
document.getElementById('progress').style.background = 'linear-gradient(to right, #0c0 ' + pc + '%, ' + ' #000 ' + pc + '%)';\
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
</script>\
  </head>\
  <body>\
\
<div class='col-xs-9'><h4>Wifi Socket Control - Maintenance</h4></div>\
 <div class='container'><div class='well'>\
 <b>Network settings</b>\
 <hr>\
 <form name='form' method='POST' action='/net' enctype='multipart/form-data'>\
  <table>\
  <tr><td>IP</td><td>");
  output += WiFi.localIP().toString();
  output += F("</td></tr>\
  <tr><td>Unit Name</td><td><input name=\"name\" value=\"");
  output += name;
  output += F("\"></td></tr>\
  <tr><td>NTP Server 1</td><td><input name=\"ntp1\" value=\"");
  output += ntp1;
  output += F("\"></td></tr>\
  <tr><td>NTP Server 2</td><td><input name=\"ntp2\" value=\"");
  output += ntp2;
  output += F("\"></td></tr>\
  <tr><td>NTP Server 3</td><td><input name=\"ntp3\" value=\"");
  output += ntp3;
  output += F("\"></td></tr>\
  <tr><td>TimeZone</td><td><select name=\"tz\">");
  for (int8_t t = -11; t <= 11; t++) {
    output += F("<option");
    if (String(t) == tz) {
      output += F(" selected");
    }
    output += F(">");
    output += t;
    output += F("</option>");
  }
  output += F("</select></td></tr>\
  <tr><td>Setup AP SSID</td><td>");
  char apname[sizeof(WIFI_SETUP_AP)+5];
  byte mac[6];
  WiFi.macAddress(mac);
  sprintf(apname, "%s%02X%02X", WIFI_SETUP_AP, mac[4], mac[5]);
  output += String(apname);
  output += F("</td></tr>\
  </table>\
  <input type='submit' value='Apply'>\
  </table>\
 </form>\
 </div></div>\
\
<div class='container'><div class='well'>\
 <b>Firmware update</b>\
 <hr>\
 Current version: ");
 output += VERSION;
 output += F("\
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
<div id='progress'></div>\
</body><html>");

  server.sendHeader("Connection", "close");
  server.sendHeader("Cache-Control", "no-store, must-revalidate");
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "text/html", output);  
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
  server.sendHeader("Refresh", "5; url=/list");
  server.send(200, "text/plain", "OK");  
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
    server.send(404, "text/plain", "FileNotFound");
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
  server.sendHeader("Refresh", "5; url=/list");
  String path;
  if(server.args() != 0) {
    path = server.arg(0);
    if(path != "/" && SPIFFS.exists(path)) {
      if (SPIFFS.remove(path)) {
        server.send(200, "text/plain", "OK");
        IDLE
        return;
      } else {
        server.send(404, "text/plain", "FileNotFound");
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
  server.send(200, "text/plain", "Rebooting...");
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
  server.send(200, "text/plain", "Settings was reset. Rebooting...");
}

void handleNetwork() {
#ifdef UPLOADPASS
  if(!server.authenticate(UPLOADUSER, UPLOADPASS)) {
    return server.requestAuthentication();
  }
#endif
  server.sendHeader("Connection", "close");
  server.sendHeader("Cache-Control", "no-store, must-revalidate");
  server.sendHeader("Refresh", "5; url=/list");
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
    name = server.arg("name");
  }
  saveConfig();
  server.send(200, "text/plain", "OK");
}
#define DEFAULT_OVERRIDE 10
void handleOverride() {
  server.sendHeader("Connection", "close");
  server.sendHeader("Cache-Control", "no-store, must-revalidate");
  //server.sendHeader("Refresh", "5; url=/list");
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
    server.on("/debug", HTTP_GET, handleDebug);
    server.begin();                                           // start to listen for clients 
    taskAdd(webHandle);
    return RUN_DELETE;
}

