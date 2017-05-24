#pragma once
enum OverrideMode { SON, SOFF, SNA };
OverrideMode operator ! (OverrideMode m) {
  if (m = SON) return SOFF;
  if (m = SOFF) return SON;
  return SNA;
}
enum LastChanged { SOCKET, GROUP };

class Override {
  public:
  OverrideMode mode;
  time_t period;
};
class Schedule {
  public:
  bool active() {
    return false;
  }
  bool active(time_t t) {
    return false;
  }
  time_t on;
  time_t off;
};
#define DEFAULT_WAVE 30
uint32_t waveTask();
class Wave: public Override {
  public:
  Wave() {
    state = true;
    period = DEFAULT_WAVE * 1000;
    taskAddWithDelay(waveTask, period);
  }
  //time_t period;
  bool state;
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
  Socket(uint8_t hwpin) {
    pin = hwpin;
    pinMode(pin, OUTPUT);
    digitalWrite(pin, LOW);
  }
  void turn(OverrideMode state) {
    if (state == SON) {
      if (wave != NULL) {
        if (wave->state == SON) {
          digitalWrite(pin, HIGH);
        }
        if (wave->state == SOFF) {
          digitalWrite(pin, LOW);
        }
      } else {
        digitalWrite(pin, HIGH);
      }
    } else {
      digitalWrite(pin, LOW);
    }
  }
  String   name;
  LastChanged overrideBy;
  OverrideMode socketOverride;
  //time_t   period;
  Override *  group;
  OverrideMode groupOverride;
  OverrideMode feedOverride;
  OverrideMode schedule;
  Wave * wave;
  private:
  uint8_t pin;
};
#define SOCKET_COUNT 8
Socket * socket[SOCKET_COUNT];

DoubleSchedule feedSchedule;

#define GROUP_COUNT 4
Override group[GROUP_COUNT];

Override feed;
uint32_t feedTask() {
  return RUN_DELETE;
}

Wave wave;
uint32_t waveTask() {
  wave.state != wave.state;
  return wave.period;
}

uint32_t socketsTask() {
  for (uint8_t i = 0; i < SOCKET_COUNT; i++) {
    bool switched = false;
    if (socket[i]->overrideBy == SOCKET) {
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
}

uint32_t initSockets() {
  for (uint8_t i = 0; i < SOCKET_COUNT; i++) {
    
  }
}
