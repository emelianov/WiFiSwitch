#pragma once
#include <Espalexa.h>

#define ID_CHAN_1           "channel one"
#define ID_CHAN_2           "channel two"
#define ID_CHAN_3           "channel three"
#define ID_CHAN_4           "channel four"
#define ID_CHAN_5           "channel five"
#define ID_CHAN_6           "channel six"
#define ID_CHAN_7           "channel seven"
#define ID_CHAN_8           "channel eight"

Espalexa espalexa;

template <int I>
void chChanged(uint8_t brightness) {
    if (brightness == 255) {  // ON
      WDEBUG("Alexa: ON CH%d\n", I);
      socket[I]->manual = SON;
    }
    else if (brightness == 0) { // OFF
      WDEBUG("Alexa: OFF CH%d\n", I);
      socket[I]->manual = SOFF;
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
    server.onNotFound([](){
      if (!espalexa.handleAlexaApiCall(server.uri(),server.arg(0))) {
        //server.send(404, "text/plain", "Not found");
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
