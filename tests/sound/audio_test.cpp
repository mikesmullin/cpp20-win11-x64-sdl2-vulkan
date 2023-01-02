// #include <cstdlib>
#include "../../src/sound/audio.hpp"

#include <iostream>
#include <stdexcept>

#include "../../src/utils/string.hpp"

int main() {
  try {
    mks::Infof("Begin sound test.");

    mks::Audio a{};
    a.addMusic("../assets/music/napoleon.ogg");
    a.playMusic(0, 999);
    a.addSoundEffect("../assets/sfx/monster_snarl.wav");

    int choice = 0;
    while (choice != -1) {
      choice = 0;
      std::cout << "Cmd> ";
      std::cin >> choice;
      if (choice != -1) {
        a.playSoundEffect(choice);
      }
      std::cin.clear();
    }

    mks::Infof("End of test.");
  } catch (const std::exception& e) {
    std::cerr << e.what() << '\n';
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}