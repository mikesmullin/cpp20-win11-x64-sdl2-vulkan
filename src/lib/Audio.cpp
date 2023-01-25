
#include "Audio.hpp"

#include <SDL.h>

#include <iostream>
#include <stdexcept>

#include "Logger.hpp"
#include "SDL.hpp"

namespace mks {

Audio::Audio() {
}

Audio::~Audio() {
}

void Audio::init() {
  const int audio_rate = 22050;
  const Uint16 audio_format = AUDIO_S16SYS;
  const int audio_channels = 2;
  const int audio_buffers = 4096;

  int ok2 = Mix_OpenAudio(audio_rate, audio_format, audio_channels, audio_buffers);
  if (ok2 != 0) {
    throw mks::Logger::Errorf("Couldn't init audio: %s", Mix_GetError());
  }
}

void Audio::addSoundEffect(const char* path) {
  Mix_Chunk* f = Mix_LoadWAV(path);
  if (f != nullptr) {
    mSoundEffectBank.push_back(f);
    unsigned int l = mSoundEffectBank.size() - 1;
    mks::Logger::Infof("Sound effect loaded. idx: %u, path: %s", l, path);
  } else {
    auto err = Mix_GetError();
    std::cout << err << std::endl;
    throw mks::Logger::Errorf(
        "Couldn't add sound effect. path: %s, error: %s",
        path,
        Mix_GetError());
  }
}

void Audio::playSoundEffect(const unsigned int id) const {
  int l = mSoundEffectBank.size() - 1;
  if (id > l) {
    throw mks::Logger::Errorf("Invalid sound efefct id: %d, max: %d", id, l);
  }

  int ok = Mix_PlayChannel(-1, mSoundEffectBank[id], 0);
  if (ok == -1) {
    throw mks::Logger::Errorf("Unable to play sound effect. id: %u, error: %s", id, Mix_GetError());
  }
  mks::Logger::Infof("Playing sound: %u", id);
}

void Audio::addMusic(const char* path) {
  Mix_Music* f = Mix_LoadMUS(path);
  if (f != nullptr) {
    mMusicBank.push_back(f);
    unsigned int l = mMusicBank.size() - 1;
    mks::Logger::Infof("Music loaded. idx: %u, path: %s", l, path);
  } else {
    throw mks::Logger::Errorf("Couldn't add music. path: %s, error: %s", path, Mix_GetError());
  }
}

void Audio::playMusic(const unsigned int id, const unsigned int loops) const {
  int l = mMusicBank.size() - 1;
  if (id > l) {
    throw mks::Logger::Errorf("Invalid music id: %d, max: %d", id, l);
  }

  int ok = Mix_PlayMusic(mMusicBank[id], loops);
  if (ok == -1) {
    throw mks::Logger::Errorf("Unable to play music. id: %u, error: %s", id, Mix_GetError());
  }
  mks::Logger::Infof("Playing music: %u", id);
}

}  // namespace mks