#include "Engine.hpp"

#include <memory>

#include "Logger.hpp"
#include "Vulkan.hpp"

namespace mks {

std::unique_ptr<Engine> Engine::Init() {
  auto e = std::make_unique<Engine>();
  e->Run();
  // TODO: uncomment after refactoring
  // auto appInfo = mks::Vulkan::DescribeApplication("Game", 1, 0, 0);
  // mks::Vulkan::CreateInstance(std::move(appInfo));
  return e;
}

Engine::Engine() {
  mks::Logger::Infof("New Engine");
}

Engine::~Engine() {
  mks::Logger::Infof("Engine deleted");
}

std::unique_ptr<Behavior> Engine::GetObject(char* name) {
  mks::Logger::Infof("Engine.GetObject()");
  auto b = std::make_unique<Behavior>();
  return b;
}

}  // namespace mks