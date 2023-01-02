#include <iostream>
#include <stdexcept>

#include "utils/string.hpp"

int main() {
  try {
    mks::Infof("Hello world!");
  } catch (const std::exception& e) {
    std::cerr << e.what() << '\n';
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}