#pragma once

#include <vulkan/vulkan.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include "Logger.hpp"

namespace mks {

class Vulkan {
 public:
  static VkApplicationInfo Vulkan::DescribeApplication(
      const char* name,
      const unsigned int major,
      const unsigned int minor,
      const unsigned int revision);

 private:
  static VkInstance instance;
};
}  // namespace mks