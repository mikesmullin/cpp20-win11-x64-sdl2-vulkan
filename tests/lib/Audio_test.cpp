#include "../../src/lib/Audio.hpp"

#include <iostream>

#include "../../src/lib/Logger.hpp"
#include "../../src/lib/SDL.hpp"

int main(int argc, char* argv[]) {
  try {
    mks::Logger::Infof("Begin Audio test.");

    mks::SDL::defaultInstance.enableAudio();
    mks::SDL::defaultInstance.init();

    mks::Audio a{};
    a.init();
    a.loadAudioFile("../assets/audio/music/retro.wav");
    a.loadAudioFile("../assets/audio/sfx/pong.wav");
    a.playAudio(0, 999);

    int choice = 0;
    while (choice != -1) {
      choice = 0;
      std::cout << "Cmd> ";
      std::cin >> choice;
      if (choice != -1) {
        a.playAudio(choice, 1);
      }
      std::cin.clear();
    }

    a.shutdown();
    mks::Logger::Infof("End of test.");
  } catch (const std::exception& e) {
    std::cerr << e.what() << '\n';
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}