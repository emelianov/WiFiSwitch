enum FeedMode {on, off, na};
class Group {
  Schedule schedule1;
  Schedule schedule2;
}
class Schedule {
  time_t on;
  time_t off;
}
class Socket {
  String   name;
  bool     on;
  Group *  group;
  FeedMode feedMode;
  Schedule schedule1;
  Schedule schedule2;
};

uint32_t initSockets() {
  
}