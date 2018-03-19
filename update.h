#pragma once
#define TAR_SILENT
#include <untar.h>
#include "StreamBuf.h"

#define FWNAME "firmware.bin"

Tar<FS> tar(&SPIFFS);
StreamBuf sb;
bool isFW = false;

// TAR Callback. Called on every file.
bool tarFile(char* b) {
  char* fwfile = FWNAME;
  if (strcmp(b, fwfile) == 0) {
    isFW = true;
    return false;
  }
  return true;
}
// TAR Callback. Called on each data block.
void tarData(char* b, size_t s) {
  if(isFW) {
    Update.write((uint8_t*)b, s);
//    if(Update.write((uint8_t*)b, s) != s){
//      Update.printError(Serial);
//    }
  }
}
// TAR Callback. Called on each file end.
void tarEof() {
  isFW = false;
}

uint32_t initUpdate(){
    sb.setTimeout(1);     // Minimize read delay
    tar.onFile(tarFile);
    tar.onData(tarData);
    tar.onEof(tarEof);
    tar.dest("/");
    server.on("/update", HTTP_POST, [](){
      #ifdef UPLOADPASS
      if(!server.authenticate(UPLOADUSER, UPLOADPASS)) {
        return server.requestAuthentication();
      }
      #endif
      server.sendHeader("Connection", "close");
      server.sendHeader("Refresh", "5; url=/list");
      server.send(200, "text/plain", (Update.hasError())?"Update failed. Rebooting...":"Update OK. Rebooting...");
      taskAddWithDelay(restartESP, 1000);
    },[](){
      #ifdef UPLOADPASS
      if(!server.authenticate(UPLOADUSER, UPLOADPASS)) {
        return server.requestAuthentication();
      }
      #endif
      HTTPUpload& upload = server.upload();
      if(upload.status == UPLOAD_FILE_START){
        //Serial.setDebugOutput(true);
        WiFiUDP::stopAll();
        //Serial.printf("Update: %s\n", upload.filename.c_str());
        uint32_t maxSketchSpace = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;
        if(!Update.begin(maxSketchSpace)){//start with max available size
          //Serial.println(maxSketchSpace);
          //Update.printError(Serial);
        }
        tar.open((Stream*)&sb);
      } else if(upload.status == UPLOAD_FILE_WRITE){
          //Serial.print("Block: ");
          //Serial.println(upload.currentSize);
          //Serial.print(".");
          sb.open((uint8_t*)&upload.buf, upload.currentSize);
          tar.extract();
          //Serial.print("o");
      }
      if(upload.status == UPLOAD_FILE_END){
        Update.end(true);
      /*
        if(Update.end(true)){ //true to set the size to the current progress
          Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
        } else {
          Update.printError(Serial);
       }
       Serial.setDebugOutput(false);
      */
      }
    });
}
