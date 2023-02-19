#pragma once

#include <memory>

#include "../components/Behavior.hpp"

namespace mks {

class Engine {
 public:
  static std::unique_ptr<Engine> Init();
  Engine();
  ~Engine();

  // void Log(char*);
  // void Info(char*);
  std::unique_ptr<Behavior> GetObject(char* name);
  void Start(void* cb){};
  void Run(){};
  void Update(){};
  void FinalUpdate(){};
  void Draw(){};
  void DrawGUI(){};
  void Stop(){};
  void Shutdown(){};
  void Bind(std::unique_ptr<Behavior> obj){};
  void TriggerSync(char* event, ...){};
  void TriggerObjectSync(char* event, std::unique_ptr<Behavior> obj, ...){};
  void Trigger(char* event, void* cb){};

 private:
};

}  // namespace mks