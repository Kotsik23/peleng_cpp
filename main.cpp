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


#include <fcntl.h>

int setNonBlocking(int fd) {
  int flags = fcntl(fd, F_GETFL, 0);
  if (flags == -1) return -1;
  return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

void reader(int readPipe, std::unique_ptr<Logger> &logger, EventTracker &tracker, std::atomic<bool> &running) {
  Event e{};
  while (running) {
    ssize_t bytesRead = read(readPipe, &e, sizeof(Event));
    if (bytesRead > 0) {
      logger->Write(e);
      tracker.Increment();
    } else if (bytesRead == 0) {
      std::cerr << "Pipe closed." << std::endl;
      break;  // Pipe was closed
    } else {
      if (errno == EAGAIN || errno == EWOULDBLOCK) {
        std::this_thread::sleep_for(
            std::chrono::milliseconds(100));
        continue;
      } else {
        std::cerr << "Error reading from pipe: " << std::strerror(errno) << std::endl;
        break;
      }
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

      ssize_t bytesWritten = write(writePipe, &e, sizeof(Event));
      if (bytesWritten == -1) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
          std::this_thread::sleep_for(std::chrono::milliseconds(100));
          continue;
        } else {
          std::cerr << "Error writing to pipe: " << strerror(errno) << std::endl;
        }
      }
    }
    std::uniform_int_distribution<> sleepDist(minEventInterval, maxEventInterval);
    std::this_thread::sleep_for(std::chrono::milliseconds(sleepDist(gen)));
  }
}


int main() {
  std::atomic<bool> running(true);
  std::atomic<bool> paused(false);
  std::atomic<int> minEventInterval(500);
  std::atomic<int> maxEventInterval(2000);

  auto logger = Logger::GetLogger(2);
  EventTracker tracker;

  CommandHandler commandHandler(running, paused, minEventInterval, maxEventInterval, logger, tracker);

  int fd[2];

  /* create the pipe */
  if (pipe(fd) == -1) {
    std::cout << "Pipe failed. " << std::strerror(errno) << std::endl;
    return 1;
  }

  setNonBlocking(fd[READ]);
  setNonBlocking(fd[WRITE]);

  std::thread tWriter(writer, fd[WRITE], std::ref(minEventInterval), std::ref(maxEventInterval), std::ref(running),
                      std::ref(paused));
  std::thread tReader(reader, fd[READ], std::ref(logger), std::ref(tracker), std::ref(running));

  commandHandler.printCommands();
  std::string input;
  while (running) {
    std::cout << "> ";
    std::getline(std::cin, input);
    commandHandler.handleCommand(input);
  }


  std::cout << "Waiting for threads to finish..." << std::endl;
  tWriter.join();
  tReader.join();

  std::cout << "All threads have finished. Exiting program." << std::endl;

  close(fd[READ]);
  close(fd[WRITE]);

  return 0;
}