#include "Window.hpp"

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <functional>
#include <thread>
#include <vector>

#include "Base.hpp"
#include "Gamepad.hpp"
#include "Keyboard.hpp"
#include "Logger.hpp"
#include "SDL.hpp"
#include "SDL_events.h"

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

  // TODO: Move to game app or lua scope
  // SDL_SetWindowMinimumSize(window, 1136, 640);   // iPhone SE 16:9
  // SDL_SetWindowMaximumSize(window, 7680, 4320);  // 8K 16:9

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

void Window::KeepAspectRatio(const u32 width, const u32 height) {
  // use the smaller of the original vs. aspect dimension
  const u32 targetWidth = std::min(static_cast<f32>(width), height * v.aspectRatio);
  const u32 targetHeight = std::min(static_cast<f32>(height), width / v.aspectRatio);

  // and then center it to provide the illusion of aspect ratio
  const u32 left = (width - targetWidth) / 2;
  const u32 top = (height - targetHeight) / 2;

  v.windowWidth = width;
  v.windowHeight = height;
  v.viewportX = left;
  v.viewportY = top;
  v.viewportWidth = targetWidth;
  v.viewportHeight = targetHeight;
  v.bufferWidth = width;
  v.bufferHeight = height;
  v.framebufferResized = true;
}

void Window::RenderLoop(
    const int physicsFps,
    const int renderFps,
    std::function<void(const float)> physicsCallback,
    std::function<void(const float)> renderCallback) {
  const std::chrono::duration<double, std::milli> physicsInterval(1000.0f / physicsFps);
  const std::chrono::duration<double, std::milli> renderInterval(1000.0f / renderFps);
  auto lastPhysics = std::chrono::high_resolution_clock::now();
  auto lastRender = std::chrono::high_resolution_clock::now();
  auto currentTime = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double, std::milli> elapsedPhysics = currentTime - lastPhysics;
  std::chrono::duration<double, std::milli> elapsedRender = currentTime - lastRender;

  float deltaTime = 0;
  u8 frameCount = 0;
  u8 fpsAvg = 0;
  char title[255];
  SDL_Event e;
  while (!quit) {
    if (!v.minimized) {
      // Physics update
      currentTime = std::chrono::high_resolution_clock::now();
      elapsedPhysics = currentTime - lastPhysics;
      if (elapsedPhysics > physicsInterval) {
        deltaTime =
            std::chrono::duration<float, std::chrono::seconds::period>(currentTime - lastPhysics)
                .count();
        lastPhysics = currentTime;

        physicsCallback(deltaTime);
      }

      // Render update
      currentTime = std::chrono::high_resolution_clock::now();
      elapsedRender = currentTime - lastRender;
      if (elapsedRender > renderInterval) {
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
                  v.maximized = false;
                  break;

                case SDL_WINDOWEVENT_MAXIMIZED:
                  v.maximized = true;
                  break;

                // case SDL_WINDOWEVENT_RESIZED:
                case SDL_WINDOWEVENT_SIZE_CHANGED:
                  v.minimized = false;
                  KeepAspectRatio(e.window.data1, e.window.data2);
                  break;
              }
              break;

            case SDL_QUIT:
              quit = true;
              break;
          }

          mks::Gamepad::OnInput(e);
          mks::Keyboard::OnInput(e);

          // SDL_UpdateWindowSurface(window);
        }

        // render
        v.AwaitNextFrame();

        currentTime = std::chrono::high_resolution_clock::now();
        deltaTime =
            std::chrono::duration<float, std::chrono::seconds::period>(currentTime - lastRender)
                .count();
        lastRender = currentTime;

        renderCallback(deltaTime);
        v.DrawFrame();

        frameCount++;
        if (frameCount >= renderFps) {
          fpsAvg = 1 / (deltaTime / frameCount);
          // if titlebar updates are tracking with the wall clock seconds hand, then loop is on-time
          // the value shown is potential frames (ie. accounts for spare cycles)
          sprintf(title, "%s | pFPS: %u", this->title, fpsAvg);
          SDL_SetWindowTitle(window, title);
          frameCount = 0;
        }
      }
    }

    // sleep to control the frame rate
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
  }
}

void Window::End() {
  SDL_DestroyWindow(window);
}

}  // namespace mks