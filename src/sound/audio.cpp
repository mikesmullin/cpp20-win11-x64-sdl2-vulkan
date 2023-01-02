
#include "audio.hpp"

#include <SDL.h>

#include <iostream>
#include <stdexcept>

#include "../utils/string.hpp"

namespace mks {

Audio::Audio() {
  int ok1 = SDL_Init(SDL_INIT_AUDIO);
  if (ok1 != 0) {
    throw mks::Errorf("Failed to SDL_Init(). error: %s", SDL_GetError());
  }

  const int audio_rate = 22050;
  const Uint16 audio_format = AUDIO_S16SYS;
  const int audio_channels = 2;
  const int audio_buffers = 4096;

  int ok2 = Mix_OpenAudio(audio_rate, audio_format, audio_channels, audio_buffers);
  if (ok2 != 0) {
    throw mks::Errorf("Couldn't init audio: %s", Mix_GetError());
  }
}

Audio::~Audio() {
  SDL_Quit();
}

void Audio::addSoundEffect(const char* path) {
  Mix_Music* f = Mix_LoadMUS(path);
  if (f != nullptr) {
    mSoundEffectBank.push_back(f);
    std::cout << (mSoundEffectBank.size() - 1) << " Sound is ready, path: " << path << std::endl;
  } else {
    auto err = Mix_GetError();
    std::cout << err << std::endl;
    throw mks::Errorf("Couldn't add sound effect: %s", err);
  }
}

void Audio::playSoundEffect(const unsigned int which) const {
  int l = mSoundEffectBank.size() - 1;
  if (which > l) {
    throw mks::Errorf("Sound out of range: %d, max: %d", which, l);
  }

  int ok = Mix_PlayMusic(mSoundEffectBank[which], 0);
  if (ok == -1) {
    throw mks::Errorf("Unable to play sound: %u", which);
  }
  mks::Infof("Played sound: %u", which);
}

}  // namespace mks