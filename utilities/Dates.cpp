#include "Dates.h"

std::string currentDate(time_t time) {
  std::tm tm = *std::localtime(&time);
  std::ostringstream oss;
  oss << std::put_time(&tm, "%Y-%m-%d");
  return oss.str();
}

std::string currentTime(time_t time) {
  std::tm tm = *std::localtime(&time);
  std::ostringstream oss;
  oss << std::put_time(&tm, "%H:%M:%S");
  return oss.str();
}