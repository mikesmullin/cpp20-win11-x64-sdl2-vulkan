
#include "../../src/lib/Gamepad.hpp"

#define SDL_MAIN_HANDLED
#include <SDL.h>

#include <iostream>
#include <stdexcept>

#include "../../src/lib/Logger.hpp"
#include "../../src/lib/SDL.hpp"

int main() {
  try {
    mks::Logger::Infof("Begin gamepad test.");

    mks::SDL::defaultInstance.enableGamepad();
    mks::Gamepad g{};
    g.init();
    mks::SDL::defaultInstance.init();

    SDL_Joystick* joystick;
    joystick = SDL_JoystickOpen(0);
    mks::Logger::Infof("Controller Name: %s\n", SDL_JoystickNameForIndex(0));

    SDL_Event event;
    int choice = 0;
    while (choice != -1) {
      choice = 0;
      SDL_WaitEvent(&event);
      if (event.type == SDL_JOYAXISMOTION) {
        mks::Logger::Infof("axis: %i %i\n", event.jaxis.axis, event.jaxis.value);
      } else if (event.type == SDL_JOYBUTTONDOWN) {
        mks::Logger::Infof("button: %i\n", event.jbutton.button);
      }

      std::cout << "Cmd> ";
      std::cin >> choice;
      if (choice != -1) {
        // a.playSoundEffect(choice);
      }
      std::cin.clear();
    }

    SDL_JoystickClose(joystick);
    mks::Logger::Infof("End of test.");
  } catch (const std::exception& e) {
    std::cerr << e.what() << '\n';
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}