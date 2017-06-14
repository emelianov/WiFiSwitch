#pragma once

#define PUMP_NONE         "100"
#define PUMP_SINGLE       "110"
#define PUMP_DOUBLE_PULSE "121"
#define PUMP_DOUBLE_ALT   "122"
#define PUMP_DOUBLE_SER   "123"
#define PUMP_DOUBLE_RND   "124"
#define PUMP_QUAD_PULSE   "141"
#define PUMP_QUAD_ALT     "142"
#define PUMP_QUAD_SER     "143"
#define PUMP_QUAD_RND     "144"

bool   dhcp = true;
String ip   = "192.168.20.99";
String mask = "255.255.255.0";
String gw   = "192.168.20.2";
String dns  = "192.168.20.2";
String ntp1 = "192.168.30.30";
String ntp2 = "192.168.30.4";
String ntp3 = "pool1.ntp.org";
String tz   = "5";
String admin = "admin";
String pass = "password";

struct configParm {
  String param;
  String * value;
};

uint32_t readConfig(configParm * parm, uint8_t count) {
  
}
//uint32_t readConfig() {
//}

