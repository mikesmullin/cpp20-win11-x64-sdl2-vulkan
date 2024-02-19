#include "../../src/lib/Gamepad.hpp"

#include <iostream>

#include "../../src/lib/Logger.hpp"

int main() {
  try {
    mks::Logger::Infof("Begin Gamepad test.");

    mks::Gamepad::Enable();
    mks::Gamepad::Init();  // not needed if you are using Window::init()

    auto joystick = mks::Gamepad{0};
    mks::Logger::Infof("Controller Name: %s", joystick.GetControllerName());
    joystick.Open();

    SDL_Event event;
    int choice = 0;
    while (choice != -1) {
      choice = 0;
      SDL_WaitEvent(&event);
      mks::Gamepad::OnInput(event);

      std::cout << "Cmd> ";
      std::cin >> choice;
      if (choice != -1) {
        // a.playAudio(choice);
      }
      std::cin.clear();
    }

    joystick.Close();

    mks::Logger::Infof("End of test.");
  } catch (const std::exception& e) {
    std::cerr << e.what() << '\n';
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}