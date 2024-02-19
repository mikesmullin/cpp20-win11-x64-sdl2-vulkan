#include "Audio.hpp"

#include <SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Logger.hpp"

namespace {

static SDL_mutex* audio_mutex;

static void lock_handler(cm_Event* e) {
  if (e->type == CM_EVENT_LOCK) {
    SDL_LockMutex(audio_mutex);
  }
  if (e->type == CM_EVENT_UNLOCK) {
    SDL_UnlockMutex(audio_mutex);
  }
}

static void audio_callback(void* udata, Uint8* stream, int size) {
  cm_process((cm_Int16*)stream, size / 2);
}

}  // namespace

namespace mks {

Audio::Audio() {
}

Audio::~Audio() {
}

void Audio::init() {
  SDL_AudioSpec fmt, got;
  cm_Source* src;

  /* Init SDL */
  SDL_Init(SDL_INIT_AUDIO);
  audio_mutex = SDL_CreateMutex();

  /* Init SDL audio */
  memset(&fmt, 0, sizeof(fmt));
  fmt.freq = 44100;
  fmt.format = AUDIO_S16;
  fmt.channels = 2;
  fmt.samples = 1024;
  fmt.callback = audio_callback;

  dev = (void*)SDL_OpenAudioDevice(NULL, 0, &fmt, &got, SDL_AUDIO_ALLOW_FREQUENCY_CHANGE);
  if (dev == 0) {
    throw mks::Logger::Errorf("Error: failed to open audio device '%s'", SDL_GetError());
  }

  /* Init library */
  cm_init(got.freq);
  cm_set_lock(lock_handler);
  cm_set_master_gain(0.5);

  /* Start audio */
  SDL_PauseAudioDevice((SDL_AudioDeviceID)dev, 0);
}

void Audio::shutdown() {
  for (const auto& src : audioSources) {
    // TODO: stop audio playback
    cm_destroy_source(src);
  }
  SDL_CloseAudioDevice((SDL_AudioDeviceID)dev);
}

void Audio::loadAudioFile(const char* path) {
  auto src = cm_new_source_from_file(path);
  if (!src) {
    throw mks::Logger::Errorf("Error: failed to load audio file '%s'\n", cm_get_error());
  }
  audioSources.push_back(src);
  mks::Logger::Infof("Audio file loaded. idx: %u, path: %s", audioSources.size() - 1, path);
}

void Audio::playAudio(const int id, const int loops) const {
  cm_set_loop(audioSources[id], loops);
  cm_play(audioSources[id]);
  // mks::Logger::Infof("Playing sound: %u", id);
}

}  // namespace mks