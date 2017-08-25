#pragma once

// Uncomment for MDNS (macOS) and LLMNR (Windows) support otherwise SSPD (Windows) is used
#define MACOS
#ifdef MACOS
#include <ESP8266mDNS.h>
#include <ESP8266LLMNR.h>
#else
#include <ESP8266SSDP.h>
#endif

uint32_t discovery() {
#ifdef MACOS
    // MDNS
    if (!MDNS.begin(name.c_str())) {
      Serial.println("Error setting up MDNS responder!");
    } else {
      MDNS.addService("http", "tcp", 80);  // Add service to MDNS-SD
      Serial.println("mDNS responder started");
    }
    // LLMNR
    LLMNR.begin(name.c_str());
    Serial.println("LLMNR reponder started");
#else
    // SSPD
    server.on("/description.xml", HTTP_GET, [](){
      SSDP.schema(http.client());
    });
    Serial.printf("Starting SSDP...\n");
    SSDP.setSchemaURL("description.xml");
    SSDP.setHTTPPort(80);
    SSDP.setName("EHSensor ModBus TCP Slave");
    SSDP.setSerialNumber(ESP.getChipId());
    SSDP.setURL("index.html");
    SSDP.setModelName("EHSensor");
    SSDP.setModelNumber("0.1");
    SSDP.setModelURL("https://github.com/emelianov/ehsensor");
    SSDP.setManufacturer("Alexander Emelianov");
    SSDP.setManufacturerURL("http://github.com/emelianov");
    SSDP.setDeviceType("urn:schemas-upnp-org:device:SensorManagement:1");
    SSDP.begin();
    Serial.printf("SSPD responder started\n");
#endif
    return RUN_DELETE;
};

