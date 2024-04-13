#include "EventTracker.h"

EventTracker::EventTracker() : count(0) {}

void EventTracker::Increment() {
  count++;
}

int EventTracker::GetCount() const {
  return count.load();
}
