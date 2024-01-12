#include "Window.hpp"

#include <cstdint>

#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_vulkan.h>

#include <chrono>
#include <thread>
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

  auto v = mks::Vulkan{"Vulkan_test", 1, 0, 0, requiredValidationLayers, requiredExtensionNames};

  v.AssertDriverValidationLayersSupported();
  v.AssertDriverExtensionsSupported(requiredExtensionNames);
  v.UsePhysicalDevice(0);

  // ask SDL to bind our Vulkan surface to the window surface
  SDL_Surface* window_surface = SDL_GetWindowSurface(window);
  if (!window_surface) {
    throw mks::Logger::Errorf("Failed to get the surface from the window");
  }
  SDL_Vulkan_CreateSurface(window, v.instance, &v.surface);
  int width, height = 0;
  SDL_Vulkan_GetDrawableSize(window, &width, &height);
  v.width = static_cast<uint32_t>(width);
  v.height = static_cast<uint32_t>(height);

  v.AssertSwapChainSupport(requiredPhysicalDeviceExtensions);
  v.UseLogicalDevice(requiredPhysicalDeviceExtensions);
  v.CreateSwapChain();
  v.CreateImageViews();
  v.CreateRenderPass();
  v.CreateDescriptorSetLayout();
  v.CreateGraphicsPipeline();
  v.CreateFrameBuffers();
  v.CreateCommandPool();
  v.CreateTextureImage();
  v.CreateTextureImageView();
  v.CreateTextureSampler();
  v.CreateVertexBuffer();
  v.CreateIndexBuffer();
  v.CreateUniformBuffers();
  v.CreateDescriptorPool();
  v.CreateDescriptorSets();
  v.CreateCommandBuffers();
  v.CreateSyncObjects();

  const int FPS = 60;  // Target frames per second
  const std::chrono::duration<double, std::milli> frameDelay(1000.0 / FPS);

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
      v.DrawFrame();
    }

    // frame rate limiter
    auto frameEnd = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> elapsedTime = frameEnd - frameStart;
    std::this_thread::sleep_for(frameDelay - elapsedTime);

    // TODO: wait remaining ms to target frame rate
  }

  v.DeviceWaitIdle();
  v.Cleanup();

  SDL_DestroyWindow(window);
}

}  // namespace mks