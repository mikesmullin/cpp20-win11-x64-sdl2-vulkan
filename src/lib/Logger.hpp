#pragma once

#include <stdexcept>

#define MKS_DEBUG_LOG

namespace mks {

class Logger {
 public:
  static std::runtime_error Errorf(const char* msg, ...);
  static std::runtime_error Errorf(const std::string msg, ...);
  static void Infof(const char* msg, ...);
  static void Debugf(const char* msg, ...);
};

}  // namespace mks