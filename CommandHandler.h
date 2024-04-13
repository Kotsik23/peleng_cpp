#ifndef TASK_COMMANDHANDLER_H
#define TASK_COMMANDHANDLER_H


#include <map>
#include <string>
#include <iostream>
#include "Logger.h"
#include "EventTracker.h"

class CommandHandler {
public:
  std::map<std::string, std::function<void(const char *)>> commands;

  CommandHandler(std::atomic<bool> &running, std::atomic<bool> &paused,
                 std::atomic<int> &minEventInterval, std::atomic<int> &maxEventInterval,
                 std::unique_ptr<Logger> &logger, EventTracker &tracker);

  void printCommands();

  void handleCommand(const std::string &input);
};

#endif //TASK_COMMANDHANDLER_H
