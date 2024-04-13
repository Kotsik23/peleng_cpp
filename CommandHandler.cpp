#include "CommandHandler.h"

CommandHandler::CommandHandler(std::atomic<bool> &running, std::atomic<bool> &paused,
                               std::atomic<int> &minEventInterval, std::atomic<int> &maxEventInterval,
                               std::unique_ptr<Logger> &logger, EventTracker &tracker) {

  commands["level"] = [&logger](const char *args) {
    if (args == nullptr) {
      std::cerr << "Error: No logging level specified." << std::endl;
      return;
    }
    int level = (int) std::strtol(args, (char **) nullptr, 10);;
    if (level < 0 || level > 2) {
      std::cerr << "Error: Invalid logging level. Valid levels are 0, 1, or 2." << std::endl;
    } else {
      logger = Logger::GetLogger(level);
    }
  };

  commands["date"] = [](const char *args) {
    auto now = std::chrono::system_clock::now();
    auto now_c = std::chrono::system_clock::to_time_t(now);
    auto localtime = std::localtime(&now_c);
    std::cout << "Current Date: "
              << localtime->tm_year + 1900
              << "." << localtime->tm_mon + 1
              << "." << localtime->tm_mday << std::endl;
  };

  commands["time"] = [](const char *args) {
    auto now = std::chrono::system_clock::now();
    auto now_c = std::chrono::system_clock::to_time_t(now);
    auto localtime = std::localtime(&now_c);
    std::cout << "Current Time: "
              << localtime->tm_hour
              << ":" << localtime->tm_min
              << ":" << localtime->tm_sec << std::endl;
  };

  commands["exit"] = [&running](const char *args) {
    running = false;
    exit(1);
  };

  commands["faster"] = [&minEventInterval, &maxEventInterval](const char *args) {
    int min = minEventInterval;
    int max = maxEventInterval;
    if (min > 1000) minEventInterval = min - 1000;
    if (max > 1000) maxEventInterval = max - 1000;
  };

  commands["slower"] = [&minEventInterval, &maxEventInterval](const char *args) {
    int min = minEventInterval;
    int max = maxEventInterval;
    minEventInterval = min + 1000;
    maxEventInterval = max + 1000;
  };

  commands["pause"] = [&paused](const char *args) {
    paused = true;
  };

  commands["resume"] = [&paused](const char *args) {
    paused = false;
  };

  commands["stat"] = [&tracker](const char *args) {
    std::cout << "Total events generated: " << tracker.GetCount() << std::endl;
  };
}

void CommandHandler::handleCommand(const std::string &input) {
  auto spacePos = input.find(' ');
  std::string command = input.substr(0, spacePos);
  const char *args = spacePos == std::string::npos ? nullptr : input.c_str() + spacePos + 1;

  if (commands.find(command) != commands.end()) {
    commands[command](args);
  } else {
    std::cerr << "Unknown command: " << command << std::endl;
  }
}

void CommandHandler::printCommands() {
  std::cout << "Available commands: " << std::endl;
  for (auto &command: commands) {
    std::cout << command.first << std::endl;
  }
  std::cout << std::endl;
}