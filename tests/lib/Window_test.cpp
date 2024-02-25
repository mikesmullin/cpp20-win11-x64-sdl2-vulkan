
#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>

#include <iostream>
#include <stdexcept>

#include "../../src/lib/Logger.hpp"
#include "../../src/lib/SDL.hpp"

int main() {
  try {
    mks::Logger::Infof("Begin Window test.");

    mks::SDL::defaultInstance.enableVideo();
    mks::SDL::defaultInstance.init();

    SDL_Window* window = SDL_CreateWindow(
        "SDL2 Window",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        680,
        480,
        0);
    if (!window) {
      std::cout << "Failed to create window\n";
      return -1;
    }

    SDL_Surface* window_surface = SDL_GetWindowSurface(window);

    if (!window_surface) {
      std::cout << "Failed to get the surface from the window\n";
      return -1;
    }

    bool quit = false;
    SDL_Event e;
    while (!quit) {
      while (SDL_PollEvent(&e) > 0) {
        switch (e.type) {
          case SDL_QUIT:
            quit = true;
            break;
        }

        SDL_UpdateWindowSurface(window);
      }
    }

    SDL_DestroyWindow(window);

    mks::Logger::Infof("End of test.");
  } catch (const std::exception& e) {
    std::cerr << e.what() << '\n';
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}