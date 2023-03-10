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

  void init();
  void addSoundEffect(const char* path);
  void playSoundEffect(const unsigned int which) const;
  void addMusic(const char* path);
  void playMusic(const unsigned int id, const unsigned int loops) const;

 private:
  std::vector<Mix_Chunk*> mSoundEffectBank;
  std::vector<Mix_Music*> mMusicBank;
};

}  // namespace mks