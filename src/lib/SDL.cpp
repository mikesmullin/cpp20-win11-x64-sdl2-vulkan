#include "SDL.hpp"

#include <SDL2/SDL.h>

#include "Logger.hpp"

namespace mks {

SDL::SDL() {
}

SDL::~SDL() {
  // mks::Logger::Infof("shutting down SDL.");
  SDL_Quit();
}

void SDL::init() {
  Uint32 flags = 0;
  if (this->useAudio) {
    flags = flags | SDL_INIT_AUDIO;
  }
  if (this->useGamepad) {
    flags = flags | SDL_INIT_JOYSTICK;
  }
  if (this->useVideo) {
    flags = flags | SDL_INIT_VIDEO;
  }
  int ok1 = SDL_Init(flags);
  if (ok1 != 0) {
    throw mks::Logger::Errorf("Failed to SDL_Init(). error: %s", SDL_GetError());
  }
}

void SDL::enableAudio() {
  this->useAudio = true;
}

void SDL::enableGamepad() {
  this->useGamepad = true;
}

void SDL::enableVideo() {
  this->useVideo = true;
}

// NOTICE: this means any process compiled with this .cpp in scope will have this instantiated
SDL SDL::defaultInstance = SDL{};

}  // namespace mks
