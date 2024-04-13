#ifndef TASK_EVENT_H
#define TASK_EVENT_H

#include <chrono>

struct Event {
  time_t timestamp;
  int id;
  int param1;
  int param2;
  int param3;
};

#endif //TASK_EVENT_H
