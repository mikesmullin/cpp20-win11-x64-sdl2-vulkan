#include <iostream>
#include <stdexcept>

#include "lib/Logger.hpp"

int main() {
  try {
    mks::Logger::Infof("Hello world!");
  } catch (const std::exception& e) {
    std::cerr << e.what() << '\n';
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}