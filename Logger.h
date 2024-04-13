#ifndef TASK_LOGGER_H
#define TASK_LOGGER_H

#include <fstream>
#include <sstream>

#include "Event.h"
#include "utilities/Dates.h"

class Logger {
protected:
  std::ofstream logFile;

public:
  explicit Logger(const std::string &fileName);

  virtual ~Logger();

  virtual void Write(const Event &event) = 0;

  static std::unique_ptr<Logger> GetLogger(int level);
};

class LoggerLevel0 : public Logger {
public:
  LoggerLevel0();

  void Write(const Event &event) override;
};

class LoggerLevel1 : public Logger {
public:
  LoggerLevel1();

  void Write(const Event &event) override;
};

class LoggerLevel2 : public Logger {
public:
  LoggerLevel2();

  void Write(const Event &event) override;
};


#endif //TASK_LOGGER_H
