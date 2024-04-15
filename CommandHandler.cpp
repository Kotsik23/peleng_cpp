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
    auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    std::cout << "Current Date: " << currentDate(now) << '\n';
  };

  commands["time"] = [](const char *args) {
    auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    std::cout << "Current Time: " << currentTime(now) << '\n';
  };

  commands["exit"] = [&running](const char *args) {
    running = false;
    std::cout << "Program was successfully exited." << '\n';
  };

  commands["faster"] = [&minEventInterval, &maxEventInterval](const char *args) {
    int minLimit = 500;
    int delta = 1000;
    if (minEventInterval > delta + minLimit) {
      minEventInterval -= delta;
      maxEventInterval -= delta;
    } else {
      std::cerr << "Cannot decrease interval further." << std::endl;
    }
  };

  commands["slower"] = [&minEventInterval, &maxEventInterval](const char *args) {
    int maxLimit = 10000;
    int delta = 1000;
    if (maxEventInterval + delta < maxLimit) {
      minEventInterval += delta;
      maxEventInterval += delta;
    } else {
      std::cerr << "Cannot increase interval further." << std::endl;
    }
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