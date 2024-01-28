#pragma once

#include <vector>

#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_vulkan.h>

#include <functional>

#include "Vulkan.hpp"

namespace mks {

class Window {
 public:
  struct DrawableArea {
    uint32_t width;
    uint32_t height;
  };

  Window();
  ~Window();

  void Begin(const char* title, const int width, const int height);
  void Bind();
  DrawableArea GetDrawableAreaExtentBounds();
  void RenderLoop(
      const int fps,
      std::function<void(SDL_Event&)> sdl_callback,
      std::function<void(float)> callback);
  void End();

  SDL_Window* window;
  Vulkan v = {};
  std::vector<const char*> requiredExtensionNames{};
};
}  // namespace mks