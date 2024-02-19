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
    a.loadAudioFile("../assets/audio/sfx/pong-01.wav");
    a.loadAudioFile("../assets/audio/sfx/pong-02.wav");
    a.loadAudioFile("../assets/audio/sfx/pong-03.wav");
    a.loadAudioFile("../assets/audio/sfx/pong-04.wav");
    a.loadAudioFile("../assets/audio/sfx/pong-05.wav");
    a.loadAudioFile("../assets/audio/sfx/pong-06.wav");
    a.loadAudioFile("../assets/audio/sfx/pong-07.wav");
    a.loadAudioFile("../assets/audio/sfx/pong-08.wav");
    a.loadAudioFile("../assets/audio/sfx/pong-09.wav");
    a.loadAudioFile("../assets/audio/sfx/pong-10.wav");
    a.loadAudioFile("../assets/audio/sfx/pong-11.wav");
    a.loadAudioFile("../assets/audio/sfx/pong-12.wav");
    a.loadAudioFile("../assets/audio/sfx/pong-13.wav");
    a.loadAudioFile("../assets/audio/sfx/pong-14.wav");
    a.loadAudioFile("../assets/audio/sfx/pong-15.wav");
    a.playAudio(0, 1);

    int choice = 0;
    while (choice != -1) {
      choice = 0;
      std::cout << "Cmd> ";
      std::cin >> choice;
      if (choice != -1) {
        a.playAudio(choice, 0);
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