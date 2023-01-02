#include "Vulkan.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <memory>

#include "Logger.hpp"

namespace mks {

const char* ENGINE_NAME = "MKS";
const unsigned int ENGINE_MAJOR = 1;
const unsigned int ENGINE_MINOR = 1;
const unsigned int ENGINE_REVISION = 1;

std::unique_ptr<VkApplicationInfo> Vulkan::DescribeApplication(
    const char* name,
    const unsigned int major,
    const unsigned int minor,
    const unsigned int revision) {
  auto appInfo = std::make_unique<VkApplicationInfo>();
  appInfo->sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  appInfo->pApplicationName = name;
  appInfo->applicationVersion = VK_MAKE_VERSION(major, minor, revision);
  appInfo->pEngineName = ENGINE_NAME;
  appInfo->engineVersion = VK_MAKE_VERSION(ENGINE_MAJOR, ENGINE_MINOR, ENGINE_REVISION);
  appInfo->apiVersion = VK_API_VERSION_1_3;
  return appInfo;
}

void Vulkan::CreateInstance(std::unique_ptr<VkApplicationInfo> appInfo) {
  auto createInfo = std::make_unique<VkInstanceCreateInfo>();
  createInfo->sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  createInfo->pApplicationInfo = appInfo.get();

  // TODO: finish
  // https://vulkan-tutorial.com/Drawing_a_triangle/Setup/Instance
  // https://wiki.libsdl.org/SDL2/SDL_Vulkan_GetInstanceExtensions
}

}  // namespace mks