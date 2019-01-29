#pragma once
#include <ESP8266Ping.h>

// Ping interval until first faled run
#define PING_DEFAULT 65000
#define PING_RETRY 10000
// Retry count before restart Wi-Fi connection
#define PING_COUNT 5

int8_t pingRetry = PING_COUNT;

uint32_t pingTask();

uint32_t replyTask() {
  if (Ping.ping() == -1) return 1000;
  if (!Ping.ping()) {
    pingRetry--;
    if (!pingRetry) {
      WDEBUG("GW failed. Restarting Wi-Fi...\n");
      pingRetry = PING_COUNT;
      taskAdd(wifiStart);
    } else {
      WDEBUG("GW failed. Attempts left: %d\n", pingRetry);
      taskAddWithDelay(pingTask, PING_RETRY);
    }
  } else {
    pingRetry = PING_COUNT;
    WDEBUG("GW is OK\n");
  }
  taskAddWithDelay(pingTask, PING_DEFAULT);
  return RUN_DELETE;
}

uint32_t pingTask() {
 #ifdef WFS_DEBUG
  String ip = WiFi.gatewayIP().toString();
  WDEBUG("Pinging %s\n", ip.c_str());
 #endif
  Ping.ping(WiFi.gatewayIP(), PING_COUNT, false);
  taskAddWithDelay(replyTask, 1000);
  return RUN_DELETE;
}

uint32_t stopPing() {
  taskDel(pingTask);
  taskDel(replyTask);
  return RUN_DELETE;
}

uint32_t initPing() {
  taskAddWithDelay(pingTask, PING_DEFAULT);
  return RUN_DELETE;
}
