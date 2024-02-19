#pragma once

#include <vector>

extern "C" {
#include "cmixer.h"
}

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
  void loadAudioFile(const char* path);
  void playAudio(const int id, const bool loop) const;
  void shutdown();

 private:
  unsigned int dev;
  std::vector<cm_Source*> audioSources;
};

}  // namespace mks