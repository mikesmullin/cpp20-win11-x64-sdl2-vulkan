#include "Logger.hpp"

#include <cstdarg>
#include <cstdio>
#include <iostream>
#include <stdexcept>

namespace mks {

std::runtime_error Logger::Errorf(const char* msg, ...) {
  char buf[255];
  va_list args;
  va_start(args, msg);
  int ok = vsprintf(buf, msg, args);
  va_end(args);
  if (ok < 1) {
    throw std::runtime_error("Unable to format error msg for logging.");
  }
  return std::runtime_error(buf);
}

void Logger::Infof(const char* msg, ...) {
  char buf[255];
  va_list args;
  va_start(args, msg);
  int ok = vsprintf(buf, msg, args);
  va_end(args);
  if (ok < 1) {
    throw std::runtime_error("Unable to format info msg for logging.");
  }
  std::cout << buf << std::endl;
}

void Logger::Debugf(const char* msg, ...) {
#ifdef MKS_DEBUG_LOG
  char buf[255];
  va_list args;
  va_start(args, msg);
  int ok = vsprintf(buf, msg, args);
  va_end(args);
  if (ok < 1) {
    throw std::runtime_error("Unable to format debug msg for logging.");
  }
  std::cout << buf << std::endl;
#endif
}

}  // namespace mks