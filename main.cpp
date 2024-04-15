#include <iostream>
#include <map>
#include <random>
#include <unistd.h>
#include <thread>

#include "Logger.h"
#include "EventTracker.h"
#include "CommandHandler.h"

#define READ  0
#define WRITE 1

void reader(int readPipe, std::unique_ptr<Logger> &logger, EventTracker &tracker, std::atomic<bool> &running,
            std::atomic<bool> &paused) {
  while (running) {
    if (!paused) {
      Event e{};
      if (read(readPipe, &e, sizeof(Event)) <= 0) {
        std::cerr << "Error reading from pipe or pipe closed. " << std::strerror(errno) << std::endl;
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


int main() {
  std::atomic<bool> running(true);
  std::atomic<bool> paused(false);
  std::atomic<int> minEventInterval(1000);
  std::atomic<int> maxEventInterval(3000);

  auto logger = Logger::GetLogger(2);
  EventTracker tracker;

  CommandHandler commandHandler(running, paused, minEventInterval, maxEventInterval, logger, tracker);

  int fd[2];

  /* create the pipe */
  if (pipe(fd) == -1) {
    std::cout << "Pipe failed. " << std::strerror(errno) << std::endl;
    return 1;
  }

  std::thread tWriter(writer, fd[WRITE], std::ref(minEventInterval), std::ref(maxEventInterval), std::ref(running),
                      std::ref(paused));
  std::thread tReader(reader, fd[READ], std::ref(logger), std::ref(tracker), std::ref(running), std::ref(paused));

  commandHandler.printCommands();
  std::string input;
  while (running) {
    std::cout << "> ";
    std::getline(std::cin, input);
    commandHandler.handleCommand(input);
  }


  tWriter.join();
  tReader.join();

  close(fd[READ]);
  close(fd[WRITE]);

  return 0;
}