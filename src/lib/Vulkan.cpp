#include "Vulkan.hpp"

namespace mks {

const char* ENGINE_NAME = "MKS";
const unsigned int ENGINE_MAJOR = 1;
const unsigned int ENGINE_MINOR = 1;
const unsigned int ENGINE_REVISION = 1;

VkApplicationInfo Vulkan::DescribeApplication(
    const char* name,
    const unsigned int major,
    const unsigned int minor,
    const unsigned int revision) {
  VkApplicationInfo appInfo{};
  appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  appInfo.pApplicationName = name;
  appInfo.applicationVersion = VK_MAKE_VERSION(major, minor, revision);
  appInfo.pEngineName = ENGINE_NAME;
  appInfo.engineVersion = VK_MAKE_VERSION(ENGINE_MAJOR, ENGINE_MINOR, ENGINE_REVISION);
  appInfo.apiVersion = VK_API_VERSION_1_3;
  return appInfo;
}

}  // namespace mks