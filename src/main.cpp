#include <cstdlib>
#include <iostream>
#include <stdexcept>

int main() {
  try {
    std::cout << "Hello world!" << std::endl;
  } catch (const std::exception& e) {
    std::cerr << e.what() << '\n';
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}