#include <iostream>
#include <chrono>
#include <map>
#include <random>
#include <atomic>
#include <string>
#include <fstream>
#include <memory>
#include <unistd.h>
#include <thread>

#define READ  0
#define WRITE 1

class EventTracker {
  std::atomic<int> count;

public:
  EventTracker() : count(0) {}

  void Increment() {
    count++;
  }

  int GetCount() const {
    return count.load();
  }
};

struct Event {
  time_t timestamp;
  int id;
  int param1;
  int param2;
  int param3;
};

class Logger {
protected:
  std::ofstream logFile;

public:
  explicit Logger(const std::string &fileName) {
    logFile.open(fileName, std::ios::app);
  }

  virtual ~Logger() {
    if (logFile.is_open()) {
      logFile.close();
    }
  }

  virtual void Write(const Event &event) = 0;

  static std::unique_ptr<Logger> GetLogger(int level);
};

class LoggerLevel0 : public Logger {
public:
  LoggerLevel0() : Logger("log_level_0.txt") {}

  void Write(const Event &event) override {
    logFile << "Time: " << std::ctime(&event.timestamp)
            << "Event ID: " << event.id << std::endl;
  }
};

class LoggerLevel1 : public Logger {
public:
  LoggerLevel1() : Logger("log_level_1.txt") {}

  void Write(const Event &event) override {
    logFile << "Time: " << std::ctime(&event.timestamp)
            << "Event ID: " << event.id
            << ", Param1: " << event.param1 << std::endl;
  }
};

class LoggerLevel2 : public Logger {
public:
  LoggerLevel2() : Logger("log_level_2.txt") {}

  void Write(const Event &event) override {
    logFile << "Time: " << std::ctime(&event.timestamp)
            << "Event ID: " << event.id
            << ", Param1: " << event.param1
            << ", Param2: " << event.param2
            << ", Param3: " << event.param3 << std::endl;
  }
};

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

void reader(int readPipe, std::unique_ptr<Logger> &logger, EventTracker &tracker, std::atomic<bool> &running,
            std::atomic<bool> &paused) {
  while (running) {
    if (!paused) {
      Event e{};
      if (read(readPipe, &e, sizeof(Event)) <= 0) {
        std::cerr << "Error reading from pipe or pipe closed. " << strerror(errno) << std::endl;
      }
      logger->Write(e);
      tracker.Increment();
    }
  }
}

void writer(int writePipe, std::atomic<int> &minEventInterval, std::atomic<int> &maxEventInterval,
            std::atomic<bool> &running,
            std::atomic<bool> &paused) {
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<> dist(1, 100);
  int eventId = 0;

  while (running) {
    if (!paused) {
      Event e{};
      auto now = std::chrono::system_clock::now();
      e.timestamp = std::chrono::system_clock::to_time_t(now);
      e.id = eventId++;
      e.param1 = dist(gen);
      e.param2 = dist(gen);
      e.param3 = dist(gen);

      if (write(writePipe, &e, sizeof(Event)) == -1) {
        std::cerr << "Error writing to pipe. " << strerror(errno) << std::endl;
      }
    }
    std::uniform_int_distribution<> sleepDist(minEventInterval, maxEventInterval);
    std::this_thread::sleep_for(std::chrono::milliseconds(sleepDist(gen)));
  }
}


void printCommands(const std::map<std::string, std::function<void(const char *)>> &commands) {
  std::cout << "Available commands: " << std::endl;
  for (auto &command: commands) {
    std::cout << command.first << std::endl;
  }
  std::cout << std::endl;
}

int main() {
  std::atomic<bool> running(true);
  std::atomic<bool> paused(false);
  std::atomic<int> minEventInterval(1000);
  std::atomic<int> maxEventInterval(3000);

  auto logger = Logger::GetLogger(2);
  EventTracker tracker;

  std::map<std::string, std::function<void(const char *)>> commands;

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

  int fd[2];

  /* create the pipe */
  if (pipe(fd) == -1) {
    std::cout << "Pipe failed. " << std::strerror(errno) << std::endl;
    return 1;
  }

  std::thread tWriter(writer, fd[WRITE], std::ref(minEventInterval), std::ref(maxEventInterval), std::ref(running),
                      std::ref(paused));
  std::thread tReader(reader, fd[READ], std::ref(logger), std::ref(tracker), std::ref(running), std::ref(paused));

  printCommands(commands);
  std::string input;
  while (running) {
    std::cout << "> ";
    std::getline(std::cin, input);
    auto spacePos = input.find(' ');
    std::string command = input.substr(0, spacePos);
    const char *args = spacePos == std::string::npos ? nullptr : input.c_str() + spacePos + 1;

    if (commands.find(command) != commands.end()) {
      commands[command](args);
    } else {
      std::cerr << "Unknown command: " << command << std::endl;
    }
  }

  tWriter.join();
  tReader.join();

  close(fd[READ]);
  close(fd[WRITE]);

  return 0;
}