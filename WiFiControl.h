#pragma once
//#define PINS D0, D1, D4, D5, D6, D7, D9, D10
// Don't use RX/TX during debug -----D9--D10
#define PINS D0, D1, D4, D5, D6, D7, D6, D7

// Define enumeration type for convinient override manipulations
enum OverrideMode { SON, SOFF, SNA };
// Override <not> operator way: !SON=SOFF, !SOFF=SON, SNA=SNA 
OverrideMode operator ! (OverrideMode m) {
  if (m == SON) return SOFF;
  if (m == SOFF) return SON;
  return SNA;
}

enum LastChanged  { SOCKET, GROUP };
enum WaveSocket   { NONE, SINGLE, DOUBLE, QUAD };
enum WaveType     { PULSE, ALTERNATIVE, SERIES, RANDOM };

// Override and task to manage override duration
class Override {
  public:
  Override(task t = NULL) {
    overrideTask = t;
  }
  bool isNa() {
    return mode == SNA;
  }
  bool isOn() {
    return mode == SON;
  }
  bool isOff() {
    return mode == SOFF;
  }
  void set(OverrideMode m) {
    mode = m;
  }
  OverrideMode get() {
    return mode;
  }
  void start(OverrideMode m, time_t t) {
    mode = m;
    period = t;
    taskDel(overrideTask);
    if (t > 0 && mode != SNA) {
      taskAddWithDelay(overrideTask, t * 1000);
    }
  }
  void stop() {
    mode = SNA;
    taskDel(overrideTask);
  }
  void on(time_t t=0) {
    start(SON, t);
  }
  void off(time_t t=0) {
    start(SOFF, t);
  }
  void na() {
    stop();
  }
  OverrideMode mode = SNA;        // Current socket/switch mode
  OverrideMode modeWaiting = SNA; // Temp mode storage
  time_t period = 0;              // Override dutation
  task overrideTask;              // Callback function
};

// Schedule class
class Schedule {
  public:
  bool act = false;
  time_t on = 0;
  time_t off = 0;
  // Schedule is active
  bool active() {
    return act;
  }
  // Time is withing schedule time range
  bool active(time_t secondsFromMidnight) {
    //time_t secondsFromMidnight = t % 86400UL;
    if (on < off)  // |   |T1|####|T2|   |
        return (secondsFromMidnight > on && secondsFromMidnight < off);
      else                  // |###|T1|   |T2|###|
        return (secondsFromMidnight > on || secondsFromMidnight < off);
  }
  void set(time_t ton, time_t toff) {
    on = ton;
    off = toff;
    act = true;
  }
};

//#define DEFAULT_WAVE 30
#define DEFAULT_WAVE 30
#define WAVE_SOC1 2
#define WAVE_SOC2 1
#define WAVE_SOC3 3
#define WAVE_SOC4 4

uint32_t wavePulse(); // Forward declaration

// Warning it's not real class
// Only one instance allowable to be used
class Wave: public Override {
  public:
  Wave() : Override(wavePulse) {
    start(SON, DEFAULT_WAVE);
  }
  WaveSocket to;
  WaveType type;
  Override waveSet[4];
};
// Compilation of two Schedule classes
class DoubleSchedule {
  public:
  bool active() {
    return schedule1.active() || schedule2.active();
  }
  bool active(time_t t) {
    bool r = false;
    if (schedule1.active()) r = r || schedule1.active(t);
    if (schedule2.active()) r = r || schedule2.active(t);
    return r;
  }
  Schedule schedule1;
  Schedule schedule2;
};
// Socket implementation
class Socket: public DoubleSchedule, public Override {
  public:
  Socket(uint8_t hwpin, task t = NULL, Wave* w = NULL): Override(t) {
    pin = hwpin;
    pinMode(pin, OUTPUT);
    digitalWrite(pin, LOW);
    wave = w;
//    times = new DoubleSchedule();
  }
  String        name = "";
  LastChanged   overrideBy = SOCKET;
  //OverrideMode  socketOverride;
  Override*     group = NULL;
  //OverrideMode  groupOverride;
  OverrideMode  feedOverride = SNA;
  OverrideMode  schedule = SNA;
  Wave*         wave = NULL;
  String        waveType = "100";      
  DoubleSchedule times;
  void turn(OverrideMode state) {
    if (state == SON) {
      if (wave != NULL) {
        if (wave->isOn()) {
          Serial.println("ON");
          digitalWrite(pin, HIGH);
        }
        if (wave->isOff()) {
          Serial.println("OFF");
          digitalWrite(pin, LOW);
        }
      } else {
        Serial.println("WAVE OFF");
        digitalWrite(pin, HIGH);
      }
    } else {
      Serial.println("OFF");
      digitalWrite(pin, LOW);
    }
  }
  void setGroup(Override* gr = NULL) {
    group = gr;
    if (group != NULL) {
      overrideBy = GROUP;
      //groupOverride = group->mode;
    } else {
      overrideBy = SOCKET;
      //groupOverride = SNA;
    }
  }
  void start(OverrideMode m, time_t t=0) {
    overrideBy = SOCKET;
    Override::start(m, t);
  }
  void setOverride(time_t t, OverrideMode m) {
    overrideBy = SOCKET;
    period = t;
    taskAddWithDelay(overrideTask, t * 1000L);
  }
  private:
  uint8_t pin;
};
#define SOCKET_COUNT 8
Socket* socket[SOCKET_COUNT];
// Template to generate callback functions for each socket
template <int I>
uint32_t socketTask() {
  Serial.println(I);
  socket[I]->mode = SNA;
  socket[I]->overrideBy = GROUP;
  return RUN_DELETE;
}
task socketTasks[SOCKET_COUNT] = { socketTask<0>, socketTask<1>, socketTask<2>, socketTask<3>, socketTask<4>, socketTask<5>, socketTask<6>, socketTask<7> };

// Feed Schedule
DoubleSchedule feedSchedule;

// Groups
#define GROUP_COUNT 4
Override* group[GROUP_COUNT];
// Template to generate callback functions for each socket
template <int I>
uint32_t groupTask() {
  group[I]->mode = SNA;
  return RUN_DELETE;
}
task groupOverride[GROUP_COUNT] = { groupTask<0>, groupTask<1>, groupTask<2>, groupTask<3> };

// Global feed
Override* feed;
uint32_t feedTask() {
  feed->na();
  return RUN_DELETE;
}

// Wave
Wave wave;
uint8_t waveSoc = 0;
uint32_t wavePulse() {
  wave.mode  = !wave.mode;
  return wave.period * 1000;
}
uint32_t waveAlt() {
  wave.mode  = !wave.mode;
  return wave.period * 1000;  
}
uint32_t waveSeries() {
/*  waveSoc++;
  if (wave.type = DOUBLE) {
    if (waveSoc >= 4) {
      waveSoc = 0;
    }    
  } else if (wave.type = QUAD) {
    if (waveSoc >= 4) {
      waveSoc = 0;
    }
  }
  */
  wave.mode  = !wave.mode;
  return wave.period * 1000;

}
uint32_t waveRandom() {
  wave.mode  = !wave.mode;
  return wave.period * 1000;
}

void setWave(WaveType t) {
    wave.type = t;
    switch (t) {
      case PULSE:
        wave.overrideTask = wavePulse;
      break;
      case ALTERNATIVE:
        wave.overrideTask = waveAlt;
      break;
      case SERIES:
        wave.overrideTask = waveSeries;
      break;
      case RANDOM:
        wave.overrideTask = waveRandom;
      break;
      default:
        wave.overrideTask = wavePulse;
    }
}
void setWave(WaveSocket t) {
    wave.to = t;
    switch (t) {
      case NONE:
        socket[WAVE_SOC1]->wave = NULL;
        socket[WAVE_SOC2]->wave = NULL;
        socket[WAVE_SOC3]->wave = NULL;
        socket[WAVE_SOC4]->wave = NULL;
      break;
      case SINGLE:
        socket[WAVE_SOC1]->wave = &wave;
        socket[WAVE_SOC2]->wave = NULL;
        socket[WAVE_SOC3]->wave = NULL;
        socket[WAVE_SOC4]->wave = NULL;
      break;
      case DOUBLE:
        socket[WAVE_SOC1]->wave = &wave;
        socket[WAVE_SOC2]->wave = &wave;
        socket[WAVE_SOC3]->wave = NULL;
        socket[WAVE_SOC4]->wave = NULL;
      break;
      case QUAD:
        socket[WAVE_SOC1]->wave = &wave;
        socket[WAVE_SOC2]->wave = &wave;
        socket[WAVE_SOC3]->wave = &wave;
        socket[WAVE_SOC4]->wave = &wave;
      break;
    }
}

// Switching sockets according to overrides, schedules and waves
// Should be run in main loop
uint32_t socketsTask() {
  for (uint8_t i = 0; i < SOCKET_COUNT; i++) {
//uint8_t i = 2;
//{
    bool switched = false;
    if (socket[i]->overrideBy == SOCKET || socket[i]->group == NULL) {
      if (socket[i]->mode != SNA) {
        Serial.print("SOCKET: ");
        Serial.println(socket[i]->group == NULL?"No grpup":"Override");
        socket[i]->turn(socket[i]->mode);
        switched = true;
      }
    } else { //.overrideBy == GROUP
      if (socket[i]->group != NULL && socket[i]->group->mode != SNA) {
        Serial.print("GROUP ");
        Serial.println(socket[i]->group->mode == SNA?"NA":"ON/OFF");
        socket[i]->turn(socket[i]->group->mode);
        switched = true;
      }
    }
    if (!switched && feed->mode != SNA) {
      Serial.println("GLOBAL FEED");
      socket[i]->turn(feed->mode);
      switched = true;
    }
    if (!switched && socket[i]->feedOverride != SNA && feedSchedule.active()) {
      Serial.println("SCHED FEED");
      if (feedSchedule.active(getTime())) {
        socket[i]->turn(socket[i]->feedOverride);
      } else {
        socket[i]->turn(!socket[i]->feedOverride);
      }
      switched = true;
    }
    if (!switched && socket[i]->active()) {
      Serial.println("SCHED SOCKET");
      if (socket[i]->active(getTime())) {
        socket[i]->turn(SON);
      } else {
        socket[i]->turn(SOFF);
      }
      switched = true;
    }
    if (!switched) {
      Serial.print("ELSE: ");
      Serial.print(socket[i]->mode);
      socket[i]->turn(socket[i]->mode);
    }
  }
  return 500;
}

void turnOffAllSockets() {
 for (uint8_t i = 0; i < SOCKET_COUNT; i++) {
  socket[i]->turn(SOFF);
 }
}
uint32_t initSockets() {
  uint8_t pins[SOCKET_COUNT] = { PINS };
  for (uint8_t i = 0; i < SOCKET_COUNT; i++) {
    socket[i] = new Socket(pins[i], socketTasks[i]);
    socket[i]->name = String(i);
  }
  for (uint8_t i = 0; i < GROUP_COUNT; i++) {
    group[i] = new Override(groupOverride[i]);
  }
  feed = new Override(feedTask);
  taskAdd(socketsTask);
  return RUN_DELETE;
}

String pump = PUMP_NONE;
void setPump(String v) {
  pump = v;
      if (v == PUMP_SINGLE) {
        setWave(SINGLE);
        setWave(PULSE);  
      } else if (v == PUMP_DOUBLE_PULSE) {
        setWave(DOUBLE);
        setWave(PULSE);
      } else if (v == PUMP_DOUBLE_ALT) {
        setWave(DOUBLE);
        setWave(ALTERNATIVE);        
      } else if (v == PUMP_DOUBLE_SER) {
        setWave(DOUBLE);
        setWave(SERIES);      
      } else if (v == PUMP_DOUBLE_RND) {
        setWave(DOUBLE);
        setWave(RANDOM);      
      } else if (v == PUMP_QUAD_PULSE) {
        setWave(QUAD);
        setWave(PULSE);        
      } else if (v == PUMP_QUAD_ALT) {
        setWave(QUAD);
        setWave(ALTERNATIVE);        
      } else if (v == PUMP_QUAD_SER) {
        setWave(QUAD);
        setWave(SERIES);              
      } else if (v == PUMP_QUAD_RND) {
        setWave(QUAD);
        setWave(RANDOM);                    
      } else {
        setWave(NONE);
        pump = PUMP_NONE;
      }
}
