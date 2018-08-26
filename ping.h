#pragma once
#include <AsyncPing.h>

// Ping interval until first faled run
#define PING_DEFAULT 60000
// Ping interval as no reply received
#define PING_FAIL 1000
// Retry count before restart Wi-Fi connection
#define PING_COUNT 5

AsyncPing ping;
volatile int8_t pingRetry = PING_COUNT;
bool reply(const AsyncPingResponse& response) {
  if (!response.answer) {
  #ifdef WFS_DEBUG
    Serial.println("GW timeout");
  #endif
    pingRetry--;
    if (pingRetry > 0) return false;
  } else {
  #ifdef WFS_DEBUG
    Serial.println("GW respond");
  #endif
    pingRetry = PING_COUNT;
  }
   return true;
}

uint32_t replyTask() {
    if (pingRetry <= 0) {
     #ifdef WFS_DEBUG
      Serial.println("GW failed");
     #endif
      pingRetry = PING_COUNT;
      ping.cancel();
      WiFi.mode(WIFI_OFF);
      taskAddWithDelay(wifiStart, WIFI_CHECK_DELAY);
    } else {
    #ifdef WFS_DEBUG
      Serial.println("GW is OK");
    #endif     
    }
    return RUN_DELETE;
}

uint32_t pingTask() {
#ifdef WFS_DEBUG
  Serial.println("Ping GW");
#endif
  ping.begin(WiFi.gatewayIP(), PING_COUNT * 2, PING_FAIL);
  taskAddWithDelay(replyTask, PING_FAIL * (PING_COUNT + 1));
  return PING_DEFAULT;
}

uint32_t initPing() {
  ping.on(true, reply);
  taskAddWithDelay(pingTask, PING_DEFAULT);
  return RUN_DELETE;
}
