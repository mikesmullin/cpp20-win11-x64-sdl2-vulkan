#pragma once

#include <vulkan/vulkan.h>

#include <memory>

namespace mks {

class Vulkan {
 public:
  static std::unique_ptr<VkApplicationInfo> Vulkan::DescribeApplication(
      const char* name,
      const unsigned int major,
      const unsigned int minor,
      const unsigned int revision);

  static void CreateInstance(std::unique_ptr<VkApplicationInfo> appInfo);

 private:
  static VkInstance instance;
};
}  // namespace mks