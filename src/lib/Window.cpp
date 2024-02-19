#include "Window.hpp"

#include <chrono>
#include <cstdint>
#include <cstdio>
#include <functional>
#include <thread>
#include <vector>

#include "Base.hpp"
#include "Logger.hpp"
#include "SDL.hpp"

namespace mks {

Window::Window() {
}
Window::~Window() {
}

// init Windows + SDL2 + Vulkan
void Window::Begin(const char* title, const int width, const int height) {
  // SDL2
  mks::SDL::defaultInstance.enableVideo();
  mks::SDL::defaultInstance.init();
  this->title = title;

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

  // list required extensions, according to SDL window manager
  unsigned int extensionCount = 0;
  SDL_Vulkan_GetInstanceExtensions(window, &extensionCount, nullptr);
  requiredExtensionNames.resize(extensionCount);
  SDL_Vulkan_GetInstanceExtensions(window, &extensionCount, requiredExtensionNames.data());
}

void Window::Bind() {
  // ask SDL to bind our Vulkan surface to the window surface
  SDL_Surface* window_surface = SDL_GetWindowSurface(window);
  if (!window_surface) {
    throw mks::Logger::Errorf("Failed to get the surface from the window");
  }
  SDL_Vulkan_CreateSurface(window, v.instance, &v.surface);
}

/**
 * for use when telling Vulkan what its drawable area (extent bounds) are, according to SDL
 * window. this may differ from what we requested, and must be less than the physical device
 * capability.
 */
Window::DrawableArea Window::GetDrawableAreaExtentBounds() {
  int dwidth, dheight = 0;
  SDL_Vulkan_GetDrawableSize(window, &dwidth, &dheight);
  return {static_cast<uint32_t>(dwidth), static_cast<uint32_t>(dheight)};
}

void Window::RenderLoop(
    const int fps,
    std::function<void(SDL_Event&)> sdl_callback,
    std::function<void(float)> callback) {
  auto startTime = std::chrono::high_resolution_clock::now();
  auto lastRun = startTime;
  auto lastMeasure = startTime;
  const std::chrono::duration<double, std::milli> frameDelay(1000.0 / fps);
  float deltaTime = 0;
  u8 frameCount = 0;
  u8 fpsAvg = 0;
  char title[255];

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

      sdl_callback(e);

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
    frameCount++;
    if (frameCount >= fps) {
      if (!v.minimized) {
        deltaTime =
            std::chrono::duration<float, std::chrono::seconds::period>(frameEnd - lastMeasure)
                .count();
        fpsAvg = 1 / (deltaTime / frameCount);
        sprintf(title, "%s | avgFPS: %u", this->title, fpsAvg);
        SDL_SetWindowTitle(window, title);
      }
      frameCount = 0;
      lastMeasure = frameEnd;
    }

    // wait remaining ms to target frame rate
    std::this_thread::sleep_for(frameDelay - elapsedTime);
  }
}

void Window::End() {
  SDL_DestroyWindow(window);
}

}  // namespace mks