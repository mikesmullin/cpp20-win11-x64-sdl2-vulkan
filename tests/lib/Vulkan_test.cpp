
#include <iostream>
#include <stdexcept>

#include "../../src/lib/Logger.hpp"
#include "../../src/lib/Window.hpp"

int main() {
  try {
    mks::Logger::Infof("Begin SDL2 Vulkan test.");

    auto w = mks::Window{};
    w.init();

    mks::Logger::Infof("End of test.");
  } catch (const std::runtime_error& e) {
    std::cerr << "Fatal: " << e.what() << '\n';
    return EXIT_FAILURE;
  } catch (...) {
    std::cerr << "Unexpected error\n";
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}