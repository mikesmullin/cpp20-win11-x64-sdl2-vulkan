#include "Window.hpp"

#include <chrono>
#include <cstdint>
#include <functional>
#include <thread>
#include <vector>

#include "Logger.hpp"
#include "SDL.hpp"

namespace mks {

Window::Window() {
}
Window::~Window() {
}

// init Windows + SDL2 + Vulkan
void Window::Begin(
    const char* vulkanAppName, const char* title, const int width, const int height) {
  // SDL2
  mks::SDL::defaultInstance.enableVideo();
  mks::SDL::defaultInstance.init();

  window = SDL_CreateWindow(
      title,
      SDL_WINDOWPOS_CENTERED,
      SDL_WINDOWPOS_CENTERED,
      width,
      height,
      SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE /* | SDL_WINDOW_SHOWN*/);
  if (!window) {
    throw mks::Logger::Errorf("Failed to create window");
  }

  const std::vector<const char*> requiredValidationLayers = {
      // all of the useful standard validation is bundled into a layer included in the SDK
      "VK_LAYER_KHRONOS_validation"};

  // list required extensions, according to SDL window manager
  unsigned int extensionCount = 0;
  SDL_Vulkan_GetInstanceExtensions(window, &extensionCount, nullptr);
  std::vector<const char*> requiredExtensionNames(extensionCount);
  SDL_Vulkan_GetInstanceExtensions(window, &extensionCount, requiredExtensionNames.data());

  const std::vector<const char*> requiredPhysicalDeviceExtensions = {
      VK_KHR_SWAPCHAIN_EXTENSION_NAME};

  v = mks::Vulkan{vulkanAppName, 1, 0, 0, requiredValidationLayers, requiredExtensionNames};

  v.AssertDriverValidationLayersSupported();
  v.AssertDriverExtensionsSupported(requiredExtensionNames);
  v.UsePhysicalDevice(0);

  // ask SDL to bind our Vulkan surface to the window surface
  SDL_Surface* window_surface = SDL_GetWindowSurface(window);
  if (!window_surface) {
    throw mks::Logger::Errorf("Failed to get the surface from the window");
  }
  SDL_Vulkan_CreateSurface(window, v.instance, &v.surface);
  int dwidth, dheight = 0;
  SDL_Vulkan_GetDrawableSize(window, &dwidth, &dheight);
  v.width = static_cast<uint32_t>(dwidth);
  v.height = static_cast<uint32_t>(dheight);

  v.AssertSwapChainSupport(requiredPhysicalDeviceExtensions);
  v.UseLogicalDevice(requiredPhysicalDeviceExtensions);
  v.CreateSwapChain();
}

void Window::RenderLoop(const int fps, std::function<void(float)> callback) {
  auto startTime = std::chrono::high_resolution_clock::now();
  auto lastRun = startTime;
  const std::chrono::duration<double, std::milli> frameDelay(1000.0 / fps);
  float deltaTime = 0;

  bool quit = false;
  SDL_Event e;
  while (!quit) {
    auto frameStart = std::chrono::high_resolution_clock::now();

    // input handling
    while (SDL_PollEvent(&e) > 0) {
      switch (e.type) {
        case SDL_WINDOWEVENT:
          switch (e.window.event) {
            case SDL_WINDOWEVENT_MINIMIZED:
              v.minimized = true;
              break;

            case SDL_WINDOWEVENT_RESTORED:
              v.minimized = false;
              break;

            // case SDL_WINDOWEVENT_MAXIMIZED:
            // case SDL_WINDOWEVENT_RESIZED:
            case SDL_WINDOWEVENT_SIZE_CHANGED:
              v.width = e.window.data1;
              v.height = e.window.data2;
              v.minimized = false;
              v.framebufferResized = true;
              break;
          }
          break;

        case SDL_QUIT:
          quit = true;
          break;
      }

      // SDL_UpdateWindowSurface(window);
    }

    // rendering
    if (!v.minimized) {
      v.AwaitNextFrame();
      deltaTime =
          std::chrono::duration<float, std::chrono::seconds::period>(frameStart - lastRun).count();
      lastRun = frameStart;
      callback(deltaTime);
      v.DrawFrame();
    }

    // frame rate limiter
    auto frameEnd = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> elapsedTime = frameEnd - frameStart;
    std::this_thread::sleep_for(frameDelay - elapsedTime);

    // TODO: wait remaining ms to target frame rate
  }
}

void Window::End() {
  v.DeviceWaitIdle();
  v.Cleanup();

  SDL_DestroyWindow(window);
}

}  // namespace mks