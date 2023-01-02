// #include <cstdlib>
#include <iostream>
#include <stdexcept>

#include "sound/audio.hpp"
#include "utils/string.hpp"

int main() {
  try {
    mks::Infof("Begin sound test.");

    // mks::Audio a;
    // a.addSoundEffect("../assets/sfx/monster_snarl.ogg");
    // a.addSoundEffect("../assets/sfx/man-scream-01.ogg");
    // a.addSoundEffect("../assets/sfx/woman-scream-01.ogg");

    // a.playSoundEffect(0);

    mks::Infof("End of test.");
  } catch (const std::exception& e) {
    std::cerr << e.what() << '\n';
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}