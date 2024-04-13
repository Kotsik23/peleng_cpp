#include "Logger.h"

Logger::Logger(const std::string &fileName) {
  logFile.open(fileName, std::ios::app);
}

Logger::~Logger() {
  if (logFile.is_open()) {
    logFile.close();
  }
}

LoggerLevel0::LoggerLevel0() : Logger("log_level_0.txt") {}

void LoggerLevel0::Write(const Event &event) {
  logFile << "Time: " << std::ctime(&event.timestamp)
          << "Event ID: " << event.id << std::endl;
}

LoggerLevel1::LoggerLevel1() : Logger("log_level_1.txt") {}

void LoggerLevel1::Write(const Event &event) {
  logFile << "Time: " << std::ctime(&event.timestamp)
          << "Event ID: " << event.id
          << ", Param1: " << event.param1 << std::endl;
}

LoggerLevel2::LoggerLevel2() : Logger("log_level_2.txt") {}

void LoggerLevel2::Write(const Event &event) {
  logFile << "Time: " << std::ctime(&event.timestamp)
          << "Event ID: " << event.id
          << ", Param1: " << event.param1
          << ", Param2: " << event.param2
          << ", Param3: " << event.param3 << std::endl;
}

std::unique_ptr<Logger> Logger::GetLogger(int level) {
  switch (level) {
    case 0:
      return std::make_unique<LoggerLevel0>();
    case 1:
      return std::make_unique<LoggerLevel1>();
    case 2:
      return std::make_unique<LoggerLevel2>();
    default:
      return nullptr;
  }
}