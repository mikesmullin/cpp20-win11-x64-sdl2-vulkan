#include "Gamepad.hpp"

#include "../../src/lib/Logger.hpp"
#include "../../src/lib/Math.hpp"
#include "../../src/lib/SDL.hpp"

namespace mks {

std::vector<Gamepad*> Gamepad::registry(0);

Gamepad::Gamepad(const uint8_t index) : index{index} {
  // NOTICE: assumes you will instantiate in order 0..3
  registry.resize(index);
  registry.push_back(this);

  axes.resize(4);
  buttons.resize(4);
}
Gamepad::~Gamepad() {
}

void Gamepad::Init() {
  SDL::defaultInstance.init();
}

bool Gamepad::enabled = false;

void Gamepad::Enable() {
  enabled = true;
  SDL::defaultInstance.enableGamepad();
}

void Gamepad::Open() {
  joystick = SDL_JoystickOpen(index);
}

const char* Gamepad::GetControllerName() {
  return SDL_JoystickNameForIndex(index);
}

void Gamepad::OnInput(const SDL_Event& event) {
  if (!enabled) {
    return;
  }

  Gamepad* g;
  switch (event.type) {
    case SDL_JOYAXISMOTION:
      g = registry[event.jdevice.which];
      g->axes[event.jaxis.axis] = static_cast<float>(
          mks::Math::map(static_cast<double>(event.jaxis.value), -32768, 32767, -1.0f, 1.0f));

      // Logger::Debugf(
      //     "JOYAXISMOTION device: %i, axis: %i, value: %i, mapped: %5.3f",
      //     event.jdevice.which,
      //     event.jaxis.axis,
      //     event.jaxis.value,
      //     g->axes[event.jaxis.axis]);
      break;

    case SDL_JOYBUTTONDOWN:
      // Logger::Debugf(
      //     "JOYBUTTONDOWN device: %i, button: %i",
      //     event.jdevice.which,
      //     event.jbutton.button);
      g = registry[event.jdevice.which];
      g->buttons[event.jbutton.button] = true;
      break;

    case SDL_JOYBUTTONUP:
      // Logger::Debugf(
      //     "JOYBUTTONUP device: %i, button: %i",
      //     event.jdevice.which,
      //     event.jbutton.button);
      g = registry[event.jdevice.which];
      g->buttons[event.jbutton.button] = false;
      break;
  }
}

void Gamepad::Close() {
  SDL_JoystickClose(joystick);
}

}  // namespace mks