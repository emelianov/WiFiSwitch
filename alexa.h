#pragma once
#include <Espalexa.h>

#define ID_CHAN_1           socket[0]->name
#define ID_CHAN_2           socket[1]->name
#define ID_CHAN_3           socket[2]->name
#define ID_CHAN_4           socket[3]->name
#define ID_CHAN_5           socket[4]->name
#define ID_CHAN_6           socket[5]->name
#define ID_CHAN_7           socket[6]->name
#define ID_CHAN_8           socket[7]->name

Espalexa espalexa;
bool alexa = false;   // If Alexa enabled

template <int I>
void chChanged(uint8_t brightness) {
    if (brightness == 255) {  // ON
      WDEBUG("Alexa: ON CH%d\n", I);
      socket[I]->alexaOn();
    }
    else if (brightness == 0) { // OFF
      WDEBUG("Alexa: OFF CH%d\n", I);
      socket[I]->alexaOff();
    }
    else {  // Value
      WDEBUG("Alexa: DIM %d CH%d\n", brightness, I);
    }
}

uint32_t alexaLoop() {
  espalexa.loop();
  return 100;
}

void anyFile(); //From web.h

uint32_t initAlexa() {
  if (!alexa) {
    server.onNotFound(anyFile);
    return RUN_DELETE;
  }
  server.onNotFound([](){
    if (!espalexa.handleAlexaApiCall(server.uri(),server.arg(0))) {
      anyFile();
    }
  });
  // Add devices
  espalexa.addDevice(ID_CHAN_1, chChanged<0>);
  espalexa.addDevice(ID_CHAN_2, chChanged<1>);
  espalexa.addDevice(ID_CHAN_3, chChanged<2>);
  espalexa.addDevice(ID_CHAN_4, chChanged<3>);
  espalexa.addDevice(ID_CHAN_5, chChanged<4>);
  espalexa.addDevice(ID_CHAN_6, chChanged<5>);
  espalexa.addDevice(ID_CHAN_7, chChanged<6>);
  espalexa.addDevice(ID_CHAN_8, chChanged<7>);

  espalexa.begin(&server);
  taskAdd(alexaLoop);
  return RUN_DELETE;
}
