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
  supported = mks::Vulkan::CheckInstanceLayers(requiredValidationLayers);
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

  supported = mks::Vulkan::CheckInstanceExtensions(requiredExtensionNames);
  if (!supported) {
    throw mks::Logger::Errorf("Vulkan driver is missing required Vulkan extensions.");
  }

  auto v = mks::Vulkan{"Vulkan_test", 1, 0, 0, requiredValidationLayers, requiredExtensionNames};

  supported = v.UsePhysicalDevice(0);
  if (!supported) {
    throw mks::Logger::Errorf("Missing required Vulkan-compatible physical device.");
  }

  SDL_Vulkan_CreateSurface(window, v.instance, &v.surface);
  int width, height = 0;
  SDL_Vulkan_GetDrawableSize(window, &width, &height);
  v.width = static_cast<uint32_t>(width);
  v.height = static_cast<uint32_t>(height);

  const std::vector<const char*> requiredPhysicalDeviceExtensions = {
      VK_KHR_SWAPCHAIN_EXTENSION_NAME};
  supported = v.CheckSwapChainSupport();
  if (!supported) {
    throw mks::Logger::Errorf("Missing swap chain support on physical device.");
  }

  v.UseLogicalDevice(requiredValidationLayers, requiredPhysicalDeviceExtensions);

  v.CreateSwapChain();
  v.CreateRenderPass();
  v.CreateGraphicsPipeline();
  v.CreateFrameBuffers();
  v.CreateCommandPool();
  v.CreateCommandBuffer();
  v.CreateSyncObjects();

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

      // SDL_UpdateWindowSurface(window);
    }

    // vulkan draw frame
    v.DrawFrame();
  }

  v.DeviceWaitIdle();

  SDL_DestroyWindow(window);
}

}  // namespace mks