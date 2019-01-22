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
    WDEBUG("GW timeout\n");
    pingRetry--;
    if (pingRetry > 0) return false;
  } else {
    WDEBUG("GW respond\n");
    pingRetry = PING_COUNT;
  }
   return true;
}

uint32_t pingTask();

uint32_t replyTask() {
    if (pingRetry <= 0) {
      WDEBUG("GW failed\n");
      pingRetry = PING_COUNT;
      ping.cancel();
      //WiFi.mode(WIFI_OFF);
      taskDel(pingTask);
      taskAddWithDelay(wifiStart, WIFI_CHECK_DELAY);
    } else {
      WDEBUG("GW is OK\n");
    }
    return RUN_DELETE;
}

uint32_t pingTask() {
#ifdef WFS_DEBUG
  WDEBUG("Pinging %s\n", WiFi.gatewayIP().toString().c_str());
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
