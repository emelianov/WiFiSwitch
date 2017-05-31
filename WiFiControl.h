#pragma once
//#define PINS D0, D1, D4, D5, D6, D7, D9, D10
// Don't use RX/TX during debug -----D9--D10
#define PINS D0, D1, D4, D5, D6, D7, D6, D7

// Define enumeration type for convinient override manipulations
enum OverrideMode { SON, SOFF, SNA };
OverrideMode operator ! (OverrideMode m) {
  if (m == SON) return SOFF;
  if (m == SOFF) return SON;
  return SNA;
}

enum LastChanged { SOCKET, GROUP };
enum WaveType { SINGLE, DOUBLE };

class Override {
  public:
  Override(task t = NULL) {
    overrideTask = t;
  }
  OverrideMode mode = SNA;
  time_t period = 0;
  task overrideTask;
  bool isNa() {
    return mode == SNA;
  }
  bool isOn() {
    return mode == SON;
  }
  bool isOff() {
    return mode == SOFF;
  }
  void start(time_t t, OverrideMode m=SON) {
    mode = m;
    period = t;
    taskAddWithDelay(overrideTask, t * 1000);
  }
  void stop() {
    mode = SNA;
    taskDel(overrideTask);
  }
};

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
  bool active(time_t t) {
    time_t secondsFromMidnight = t % 86400UL;
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
#define DEFAULT_WAVE 3
uint32_t waveTask();
// Warning it's not real class
// Only one instance allowable to be used
class Wave: public Override {
  public:
  Wave() : Override(waveTask) {
    start(DEFAULT_WAVE);
  }
  void setWaveType(WaveType t) {
    switch (t) {
      case SINGLE:
      break;
      case DOUBLE:
      break;
    }
  }
  WaveType type;
};
class DoubleSchedule {
  public:
  bool active() {
    return schedule1.active() || schedule2.active();
  }
  bool active(time_t t) {
    return schedule1.active(t) || schedule2.active(t);
  }
  Schedule schedule1;
  Schedule schedule2;
};
class Socket: public DoubleSchedule, public Override {
  public:
  Socket(uint8_t hwpin, task t = NULL, Wave* w = NULL): Override(t) {
    pin = hwpin;
    pinMode(pin, OUTPUT);
    digitalWrite(pin, LOW);
    wave = w;
  }
  String        name;
  LastChanged   overrideBy;
  OverrideMode  socketOverride;
  Override*     group;
  OverrideMode  groupOverride;
  OverrideMode  feedOverride;
  OverrideMode  schedule;
  Wave*         wave;

  void turn(OverrideMode state) {
    if (state == SON) {
      
      if (wave != NULL) {
        if (wave->isOn()) {
          digitalWrite(pin, HIGH);
        }
        if (wave->isOff()) {
          digitalWrite(pin, LOW);
        }
      } else {
        digitalWrite(pin, HIGH);
      }
    } else {
      digitalWrite(pin, LOW);
    }
  }
  void assignGroup(Override * gr = NULL) {
    group = gr;
    if (group != NULL) {
      overrideBy = GROUP;
      groupOverride = group->mode;
    } else {
      overrideBy = SOCKET;
      groupOverride = SNA;
    }
  }
  private:
  uint8_t pin;
};
#define SOCKET_COUNT 8
Socket* socket[SOCKET_COUNT];
template <int I>
uint32_t socketTask() {
  socket[I]->socketOverride = SNA;
  return RUN_DELETE;
}
task socketTasks[SOCKET_COUNT] = { socketTask<0>, socketTask<1>, socketTask<2>, socketTask<3>, socketTask<4>, socketTask<5>, socketTask<6>, socketTask<7> };

DoubleSchedule feedSchedule;

#define GROUP_COUNT 4
Override* group[GROUP_COUNT];
template <int I>
uint32_t groupTask() {
  group[I]->mode = SNA;
  return RUN_DELETE;
}
task groupOverride[GROUP_COUNT] = { groupTask<0>, groupTask<1>, groupTask<2>, groupTask<3> };
Override feed;
uint32_t feedTask() {
  return RUN_DELETE;
}

Wave wave;
uint32_t waveTask() {
  wave.mode = !wave.mode;
  return wave.period * 1000;
}

uint32_t socketsTask() {
  for (uint8_t i = 0; i < SOCKET_COUNT; i++) {
    bool switched = false;
    if (socket[i]->overrideBy == SOCKET || socket[i]->group == NULL) {
      if (socket[i]->socketOverride != SNA) {
        socket[i]->turn(socket[i]->socketOverride);
        switched = true;
      }
    } else { //.overrideBy == GROUP
      if (socket[i]->group != NULL && socket[i]->group->mode != SNA) {
        socket[i]->turn(socket[i]->group->mode);
        switched = true;
      }
    }
    if (!switched && feed.mode != SNA) {
      socket[i]->turn(feed.mode);
      switched = true;
    }
    if (!switched && socket[i]->feedOverride != SNA) {
      if (feedSchedule.active(getTime())) {
        socket[i]->turn(socket[i]->feedOverride);
      } else {
        socket[i]->turn(!socket[i]->feedOverride);
      }
      switched = true;
    }
    if (!switched && socket[i]->active()) {
      if (socket[i]->active(getTime())) {
        socket[i]->turn(SON);
      } else {
        socket[i]->turn(SOFF);
      }
      switched = true;
    }
  }
  return 500;
}
void setWave(WaveType t) {
  
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
  taskAdd(socketsTask);
  return RUN_DELETE;
}
