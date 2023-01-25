#pragma once

namespace mks {

class SDL {
 public:
  SDL();
  ~SDL();

  void init();
  void enableAudio();
  void enableGamepad();
  void enableVideo();

  static SDL defaultInstance;

 private:
  bool useAudio;
  bool useGamepad;
  bool useVideo;
};

}  // namespace mks