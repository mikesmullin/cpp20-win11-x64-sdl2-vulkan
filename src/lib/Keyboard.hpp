#pragma once

#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>

#include "Base.hpp"

namespace mks {

struct KeyboardState {
  bool pressed = false;
  bool altKey = false;
  bool ctrlKey = false;
  bool shiftKey = false;
  bool metaKey = false;
  u8 code = 0;
  u8 location = 0;
};

class Keyboard {
 public:
  static KeyboardState state;
  static void OnInput(const SDL_Event& event);
};

}  // namespace mks