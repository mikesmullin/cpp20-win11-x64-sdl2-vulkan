#include <iostream>
#include <stdexcept>

#include "utils/log.hpp"

int main() {
  try {
    mks::Log::Infof("Hello world!");
  } catch (const std::exception& e) {
    std::cerr << e.what() << '\n';
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}