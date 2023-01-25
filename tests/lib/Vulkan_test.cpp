
#include "../../src/lib/Vulkan.hpp"

#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_vulkan.h>

#include <iostream>
#include <stdexcept>
#include <vector>

#include "../../src/lib/Logger.hpp"
#include "../../src/lib/SDL.hpp"

int main() {
  try {
    mks::Logger::Infof("Begin SDL2 Vulkan test.");

    mks::SDL::defaultInstance.enableVideo();
    mks::SDL::defaultInstance.init();

    SDL_Window* window = SDL_CreateWindow(
        "SDL2 Vulkan Window",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        800,
        600,
        SDL_WINDOW_VULKAN /* | SDL_WINDOW_SHOWN*/);
    if (!window) {
      throw mks::Logger::Errorf("Failed to create window");
    }

    SDL_Surface* window_surface = SDL_GetWindowSurface(window);

    if (!window_surface) {
      throw mks::Logger::Errorf("Failed to get the surface from the window");
    }

    // TODO: bind vulkan to sdl window handle
    auto v = mks::Vulkan{};
    auto appInfo = v.DescribeApplication("Vulkan_test", 1, 0, 0);
    unsigned int extensionCount = 0;
    SDL_Vulkan_GetInstanceExtensions(window, &extensionCount, nullptr);
    std::vector<const char*> extensionNames(extensionCount);
    SDL_Vulkan_GetInstanceExtensions(window, &extensionCount, extensionNames.data());

    auto supported = v.CheckSupportedExtensions(extensionNames);
    if (!supported) {
      throw mks::Logger::Errorf("Missing required Vulkan extensions.");
    }

    v.CreateInstance(std::move(appInfo), extensionNames);

    bool quit = false;
    SDL_Event e;
    while (!quit) {
      while (SDL_PollEvent(&e) > 0) {
        switch (e.type) {
            // TODO: implement window resizing

          case SDL_QUIT:
            quit = true;
            break;
        }

        // TODO: execute vulkan pipeline

        SDL_UpdateWindowSurface(window);
      }
    }

    SDL_DestroyWindow(window);

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