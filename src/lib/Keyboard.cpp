#include "Keyboard.hpp"

namespace mks {

KeyboardState Keyboard::state{};

void Keyboard::OnInput(const SDL_Event& e) {
  switch (e.type) {
    case SDL_KEYDOWN:
    case SDL_KEYUP:
      switch (e.key.state) {
        case SDL_PRESSED:
          state.pressed = true;
          break;
        case SDL_RELEASED:
        default:
          state.pressed = false;
          break;
      }
      state.altKey = e.key.keysym.mod & (KMOD_LALT | KMOD_RALT);
      state.ctrlKey = e.key.keysym.mod & (KMOD_LCTRL | KMOD_RCTRL);
      state.shiftKey = e.key.keysym.mod & (KMOD_LSHIFT | KMOD_RSHIFT);
      state.metaKey = e.key.keysym.mod & (KMOD_LGUI | KMOD_RGUI);
      // e.key.repeat;
      state.code = e.key.keysym.scancode;  // for WASD, arrow keys
      state.location = e.key.keysym.sym;   // for everything else
      break;
  }
}

}  // namespace mks