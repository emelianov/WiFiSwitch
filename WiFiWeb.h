#pragma once

#define INDEX "/index.htm"
#define UPLOADUSER  "admin"
#define UPLOADPASS "password"
//#undef UPLOADPASS
#define BUSY ;
#define IDLE ;
#define SAVE_DELAY 5000

ESP8266WebServer server(80);      // create a server at port 80

// Determinating conternt type header attribute depending on file extension
String getContentType(String filename) {
  if(server.hasArg("download")) return "application/octet-stream";
  else if(filename.endsWith(".htm")) return "text/html";
  else if(filename.endsWith(".html")) return "text/html";
  else if(filename.endsWith(".css")) return "text/css";
  else if(filename.endsWith(".js")) return "application/javascript";
  else if(filename.endsWith(".png")) return "image/png";
  else if(filename.endsWith(".gif")) return "image/gif";
  else if(filename.endsWith(".jpg")) return "image/jpeg";
  else if(filename.endsWith(".ico")) return "image/x-icon";
  else if(filename.endsWith(".xml")) return "text/xml";
  else if(filename.endsWith(".pdf")) return "application/x-pdf";
  else if(filename.endsWith(".zip")) return "application/x-zip";
  else if(filename.endsWith(".gz")) return "application/x-gzip";
  return "text/plain";
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
        socket[i]->on();
      } else {
        socket[i]->na();
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
      if (pump != v)
        setPump(v),
        wave.on(wave.period);
    }
    save = true;
  }
  {
    String cArg = "W";
    if (server.hasArg(cArg)) {
      time_t t = strToTime24(server.arg(cArg));
      if (wave.period != t)
        wave.on(t/600);
      save = true;
    }
  }
  String res = "";
  sprintf_P(data, PSTR("<?xml version = \"1.0\" ?>\n<state>\n<analog>%d</analog>\n"), analogRead(A0));
  res += data;
  //Global feed mode
  sprintf_P(data, PSTR("<Switch>%s</Switch><Override>%lu</Override><Waiting>%s</Waiting>"),
              (feed->mode==SON)?"on":(feed->mode==SOFF)?"off":"default",
              taskRemainder(feedTask)/1000,
              (feed->modeWaiting==SON)?"on":(feed->modeWaiting==SOFF)?"off":"default"
              );
  res += data;
  for (i = 0; i < GROUP_COUNT; i++) {
    sprintf_P(data, PSTR("<Switch>%s</Switch><Override>%lu</Override><Waiting>%s</Waiting>"),
              (group[i]->mode==SON)?"on":(group[i]->mode==SOFF)?"off":"default",
              taskRemainder(groupOverride[i])/1000,
              (group[i]->modeWaiting==SON)?"on":(group[i]->modeWaiting==SOFF)?"off":"default"
              );
    res += data;
  }
  // Socket switch state
  for (i = 0; i < SOCKET_COUNT; i++) {
    sprintf_P(data, PSTR("<Socket>%s</Socket>\n"), socket[i]->isOn()?"checked":"unckecked");
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
    sprintf_P(data, PSTR("<TimerCheckbox>%s</TimerCheckbox>\n<TimerCheckbox>%s</TimerCheckbox>\n<TimerValue>%s</TimerValue>\n<TimerValue>%s</TimerValue>\n<TimerValue>%s</TimerValue>\n<TimerValue>%s</TimerValue>\n<Group>%d</Group>\n<Override>%lu</Override>"),
              socket[i]->schedule1.active()?"checked":"unckecked",
              socket[i]->schedule2.active()?"checked":"unckecked",
              timeToStr(socket[i]->schedule1.on).c_str(),
              timeToStr(socket[i]->schedule1.off).c_str(),
              timeToStr(socket[i]->schedule2.on).c_str(),
              timeToStr(socket[i]->schedule2.off).c_str(),
              gr,
              taskRemainder(socketTasks[i]) / 1000
              );
    res += data;
  }
  for (i = 0; i < SOCKET_COUNT; i++) {
    sprintf_P(data, PSTR("<Switch>%s</Switch><Waiting>%s</Waiting>"),
              (socket[i]->mode==SON)?"on":(socket[i]->mode==SOFF)?"off":"default",
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
  sprintf_P(data, PSTR("<TimerCheckbox>%s</TimerCheckbox>\n<TimerCheckbox>%s</TimerCheckbox>\n<TimerValue>%s</TimerValue>\n<TimerValue>%s</TimerValue>\n<TimerValue>%s</TimerValue>\n<TimerValue>%s</TimerValue>\n"),
              feedSchedule.schedule1.active()?"checked":"unckecked",
              feedSchedule.schedule2.active()?"checked":"unckecked",
              timeToStr(feedSchedule.schedule1.on).c_str(),
              timeToStr(feedSchedule.schedule1.off).c_str(),
              timeToStr(feedSchedule.schedule2.on).c_str(),
              timeToStr(feedSchedule.schedule2.off).c_str()
              );
  res += data;
  sprintf_P(data, PSTR("<Wave>%s</Wave><Pump>%s</Pump>"),
              timeToStr24(wave.period*600).c_str(), pump.c_str());
  res += data;
  res += "</state>";
  server.send(200, "text/xml", res);                      // Send string as XML document to cliend.
                                                          // 200 - means Success html result code
  if (save) {           // If save flag set true queue save state
    taskDel(saveState); // Remove previous save request if any
    taskAddWithDelay(saveState, SAVE_DELAY);  // save in SAVE_DELAY mS
  }
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
  String output = "<html><head><meta charset='utf-8'>\
  <title>ESP8266 - File operations</title>\
  <body>\
  <form method='POST' action='/edit' enctype='multipart/form-data'>\
  Upload file to local filesystem:<br>\
   <input type='file' name='update'>\
   <input type='submit' value='Upload file'>\
  </form>";
  String path = server.hasArg("dir")?server.arg("dir"):"/";
  Dir dir = SPIFFS.openDir(path);
  while(dir.next()){
    File entry = dir.openFile("r");
    String filename = String(entry.name());
    output += "<br>";
    output += "<a href='" + filename + "'>" + filename + "</a>&nbsp<a href='/delete?file=" + filename + "'><font color=red>delete</font></a>";
    output += "<br>";
    entry.close();
  }
  output += "</body><html>";
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
void handleFileUpload(){
  File fsUploadFile;
  Serial.println("UPLOAD");
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
  String contentType = getContentType(path);
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
  Serial.println("Init WebServer");
    server.on("/ajax_inputs", HTTP_GET, ajaxInputs);  // call function ajaxInputs() if Web Server gets request http://192.168.1.20/ajax_inputs?LED1=0...
    // You can add multiple server.on(url...) to handle different url by specific routines
    server.on(INDEX, HTTP_GET, indexFile); // call function indexFile() on GET <INDEX>
    server.on("/list", HTTP_GET, listFile);                   // List/Upload/Delete page
    server.on("/delete", HTTP_GET, handleDelete);                   // Delete File
    server.on("/edit", HTTP_POST, handleFile, handleFileUpload);    // Upload file
    server.onNotFound(anyFile);         // call function anyFile() on any other requests
    server.on("/socket", HTTP_GET, handleOverride);
    server.begin();                     // start to listen for clients 
    taskAdd(webHandle);
    return RUN_DELETE;
}

