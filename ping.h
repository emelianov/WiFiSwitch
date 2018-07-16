#pragma once
#include <ESP8266Ping.h>

// Ping interval until first faled run
#define PING_DEFAULT 60000
// Ping interval as no reply received
#define PING_FAIL 1000
// Retry count before restart Wi-Fi connection
#define PING_COUNT 5

int8_t pingRetry = PING_COUNT;

uint32_t ping() {
  if(!Ping.ping(WiFi.gatewayIP(), 1)) {
    pingRetry--;
    if (!pingRetry) {
     #ifdef WFS_DEBUG
      Serial.println("GW failed");
     #endif
      pingRetry = PING_COUNT;
      WiFi.mode(WIFI_OFF);
      taskAddWithDelay(wifiStart, WIFI_CHECK_DELAY);
      return RUN_DELETE;
    }
    return PING_FAIL;
  }
 #ifdef WFS_DEBUG
  Serial.println("GW is OK");
 #endif
  return PING_DEFAULT;
}

uint32_t initPing() {
  taskAddWithDelay(ping, PING_DEFAULT);
  return RUN_DELETE;
}
