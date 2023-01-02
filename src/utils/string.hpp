#pragma once

#include <stdexcept>

namespace mks {

std::runtime_error Errorf(const char* msg, ...);
void Infof(const char* msg, ...);

}  // namespace mks