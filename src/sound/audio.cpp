
#include "audio.hpp"

#include <SDL.h>

#include <iostream>
#include <stdexcept>

#include "../utils/string.hpp"

namespace mks {

const int audio_rate = 22050;
const Uint16 audio_format = AUDIO_S16SYS;
const int audio_channels = 2;
const int audio_buffers = 4096;

Audio::Audio() {
  // SDL_Init(SDL_INIT_AUDIO);

  // int result = Mix_OpenAudio(audio_rate, audio_format, audio_channels, audio_buffers);
  // if (result != 0) {
  //   throw mks::Errorf("Couldn't init audio: %s", Mix_GetError());
  // }
}

Audio::~Audio() {
  // SDL_Quit();
}

void Audio::addSoundEffect(const char* path) {
  // Mix_Chunk* tmpChunk = Mix_LoadWAV(path);
  // if (tmpChunk != nullptr) {
  //   mSoundEffectBank.push_back(tmpChunk);
  //   std::cout << (mSoundEffectBank.size() - 1) << " Sound is ready, path: " << path << std::endl;
  // } else {
  //   throw mks::Errorf("Couldn't add sound effect: %s", Mix_GetError());
  // }
}

void Audio::playSoundEffect(const int which) const {
  // int l = mSoundEffectBank.size() - 1;
  // if (which > l) {
  //   throw mks::Errorf("Sound out of range: %d, max: %d", which, l);
  // }

  // Mix_PlayChannel(-1, mSoundEffectBank[which], 0);
  // mks::Infof("Played sound: %d", which);
}

}  // namespace mks