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
  void KeepAspectRatio(const u32 width, const u32 height);
  void RenderLoop(
      const int physicsFps,
      const int renderFps,
      std::function<void(const float)> physicsCallback,
      std::function<void(const float)> renderCallback);
  void End();

  bool quit = false;
  SDL_Window* window;
  const char* title;
  Vulkan v = {};
  std::vector<const char*> requiredExtensionNames;
};
}  // namespace mks