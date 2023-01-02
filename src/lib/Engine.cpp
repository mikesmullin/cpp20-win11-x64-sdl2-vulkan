#include "Engine.hpp"

#include "Logger.hpp"
#include "Vulkan.hpp"

namespace mks {

void Engine::Init() {
  auto appInfo = mks::Vulkan::DescribeApplication("Game", 1, 0, 0);
}

}  // namespace mks