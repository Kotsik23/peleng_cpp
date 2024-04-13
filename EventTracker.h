#ifndef TASK_EVENTTRACKER_H
#define TASK_EVENTTRACKER_H

#include <atomic>

class EventTracker {
  std::atomic<int> count;

public:
  EventTracker();

  void Increment();

  int GetCount() const;
};


#endif //TASK_EVENTTRACKER_H
