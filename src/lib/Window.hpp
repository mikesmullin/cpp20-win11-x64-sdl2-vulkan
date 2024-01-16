#pragma once

#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_vulkan.h>

#include <functional>

#include "Vulkan.hpp"

namespace mks {
class Window {
 public:
  Window();
  ~Window();

  void Begin(const char* vulkanAppName, const char* title, const int width, const int height);
  void RenderLoop(const int fps, std::function<void()> callback);
  void End();

  SDL_Window* window;
  Vulkan v = {};
};
}  // namespace mks