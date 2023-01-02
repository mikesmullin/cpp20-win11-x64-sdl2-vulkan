#pragma once

#include <SDL_mixer.h>

#include <vector>

/*
AudioListener & AudioMixer --> AudioSource --> AudioClip
see: https://docs.unity3d.com/Manual/class-AudioSource.html
*/

namespace mks {

class Audio {
 public:
  Audio();
  ~Audio();

  void addSoundEffect(const char* path);
  void playSoundEffect(const unsigned int which) const;

 private:
  std::vector<Mix_Music*> mSoundEffectBank;
};

}