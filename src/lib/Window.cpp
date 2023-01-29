#include "Window.hpp"

#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_vulkan.h>

#include <set>
#include <vector>

#include "Logger.hpp"
#include "SDL.hpp"
#include "Vulkan.hpp"

namespace mks {

Window::Window() {
}
Window::~Window() {
}

void Window::init() {
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
  bool supported = false;

#ifdef DEBUG_VULKAN
  // list required validation layers, according to developer debug preferences
  const std::vector<const char*> requiredValidationLayers = {
      // all of the useful standard validation is bundled into a layer included in the SDK
      "VK_LAYER_KHRONOS_validation"};
  supported = mks::Vulkan::CheckLayers(requiredValidationLayers);
  if (!supported) {
    throw mks::Logger::Errorf("Missing required Vulkan validation layers.");
  }

  // TODO: Device-specific layer validation is deprecated. However,
  //   the specification document still recommends that you enable validation layers at device
  //   level as well for compatibility, and it's required by some implementations. we'll see this
  //   code later on:
  //   https://vulkan-tutorial.com/en/Drawing_a_triangle/Setup/Logical_device_and_queues
#endif

  // list required extensions, according to SDL window manager
  unsigned int extensionCount = 0;
  SDL_Vulkan_GetInstanceExtensions(window, &extensionCount, nullptr);
  std::vector<const char*> requiredExtensionNames(extensionCount);
  SDL_Vulkan_GetInstanceExtensions(window, &extensionCount, requiredExtensionNames.data());
  const std::vector<const char*> deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

  supported = mks::Vulkan::CheckExtensions(requiredExtensionNames);
  if (!supported) {
    throw mks::Logger::Errorf("Missing required Vulkan extensions.");
  }

  auto v = mks::Vulkan{"Vulkan_test", 1, 0, 0, requiredValidationLayers, requiredExtensionNames};

  supported = v.UsePhysicalDevice(0);
  if (!supported) {
    throw mks::Logger::Errorf("Missing required Vulkan-compatible physical device.");
  }

  SDL_Vulkan_CreateSurface(window, v.instance, &v.surface);
  v.UseLogicalDevice(requiredValidationLayers, {&v.pdqfs.graphics, &v.pdqfs.present});

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
}

}  // namespace mks