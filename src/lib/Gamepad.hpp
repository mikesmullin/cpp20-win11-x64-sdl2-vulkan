#pragma once

#define SDL_MAIN_HANDLED
#include <SDL.h>

#include <vector>

namespace mks {

class Gamepad {
 public:
  static std::vector<Gamepad*> registry;
  const uint8_t index;
  SDL_Joystick* joystick;
  static bool enabled;
  std::vector<float> axes;
  std::vector<bool> buttons;

  Gamepad(const uint8_t index);
  ~Gamepad();

  static void Init();
  static void Enable();
  void Open();
  const char* GetControllerName();
  static void OnInput(const SDL_Event& event);
  void Close();
};

}  // namespace mks