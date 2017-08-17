#pragma once
#include <untar.h>
#include "StreamBuf.h"

const char* serverIndex = "<form method='POST' action='/update' enctype='multipart/form-data'><input type='file' name='update'><input type='submit' value='Update'></form>";
Tar<FS> tar(&SPIFFS);
StreamBuf sb;
char* fwfile = "firmware.bin";
bool isFW = false;

bool tarFile(char* b) {
  if (strcmp(b, fwfile) == 0) {
    isFW = true;
    return false;
  }
  return true;
}

void tarData(char* b, size_t s) {
  if(isFW) {
    if(Update.write((uint8_t*)b, s) != s){
      Update.printError(Serial);
    }
  }
}

void tarEof() {
  isFW = false;
  Serial.println("EOF");
  //if(Update.end(true)){ //true to set the size to the current progress
  //  Serial.printf("Update Success: %u\nRebooting...\n", 0);
  //} else {
  //  Update.printError(Serial);
  //}
  //Serial.setDebugOutput(false);
}

uint32_t initUpdate(){
    tar.onFile(tarFile);
    tar.onData(tarData);
    tar.onEof(tarEof);
    tar.dest("/");
    server.on("/update", HTTP_POST, [](){
      server.sendHeader("Connection", "close");
      server.send(200, "text/plain", (Update.hasError())?"FAIL":"OK");
      ESP.restart();
    },[](){
      HTTPUpload& upload = server.upload();
      if(upload.status == UPLOAD_FILE_START){
        Serial.setDebugOutput(true);
        WiFiUDP::stopAll();
        Serial.printf("Update: %s\n", upload.filename.c_str());
        uint32_t maxSketchSpace = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;
        if(!Update.begin(maxSketchSpace)){//start with max available size
          Serial.println(maxSketchSpace);
          Update.printError(Serial);
        }
        tar.open((Stream*)&sb);
      } else if(upload.status == UPLOAD_FILE_WRITE){
          //Serial.print("Block: ");
          //Serial.println(upload.currentSize);
          Serial.print(".");
          sb.open((uint8_t*)&upload.buf, upload.currentSize);
          tar.extract();
          //if(Update.write(upload.buf, upload.currentSize) != upload.currentSize){
          //  Update.printError(Serial);
          //}
      }
      if(upload.status == UPLOAD_FILE_END){
        if(Update.end(true)){ //true to set the size to the current progress
          Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
        } else {
          Update.printError(Serial);
       }
        Serial.setDebugOutput(false);
      }
    });
}
