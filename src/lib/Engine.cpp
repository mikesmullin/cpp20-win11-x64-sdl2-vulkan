#include "Engine.hpp"

namespace mks {

Engine Engine::Init() {
  Engine e{};
  auto appInfo = mks::Vulkan::DescribeApplication("Game", 1, 0, 0);
  return e;
}

Engine::Engine() {
  mks::Logger::Infof("New Engine");
}

Engine::~Engine() {
  mks::Logger::Infof("Engine deleted");
}

Behavior Engine::GetObject(char* name) {
  mks::Logger::Infof("Engine.GetObject()");
  Behavior b{};
  return b;
}

}  // namespace mks