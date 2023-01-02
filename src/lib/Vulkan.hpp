#pragma once

#include <vulkan/vulkan.h>

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