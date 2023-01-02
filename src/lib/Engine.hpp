#pragma once

#include "../components/Behavior.hpp"
#include "Logger.hpp"
#include "Vulkan.hpp"

namespace mks {

class Engine {
 public:
  static Engine Engine::Init();
  Engine::Engine();
  Engine::~Engine();

  // void Log(char*);
  // void Info(char*);
  Behavior GetObject(char* name);
  void Start(void* cb){};
  void Run(){};
  void Update(){};
  void FinalUpdate(){};
  void Draw(){};
  void DrawGUI(){};
  void Stop(){};
  void Shutdown(){};
  void Bind(Behavior* obj){};
  void TriggerSync(char* event, ...){};
  void TriggerObjectSync(char* event, Behavior* obj, ...){};
  void Trigger(char* event, void* cb){};

 private:
};

}  // namespace mks