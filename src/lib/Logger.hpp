#pragma once

#include <stdexcept>

namespace mks {

class Logger {
 public:
  static std::runtime_error Errorf(const char* msg, ...);
  static void Infof(const char* msg, ...);
};

}  // namespace mks