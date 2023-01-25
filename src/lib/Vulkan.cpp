#include "Vulkan.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <memory>
#include <vector>

#include "Logger.hpp"

// TODO: finish
// https://vulkan-tutorial.com/Drawing_a_triangle/Setup/Instance
// https://wiki.libsdl.org/SDL2/SDL_Vulkan_GetInstanceExtensions

namespace mks {

const char* ENGINE_NAME = "MKS";
const unsigned int ENGINE_MAJOR = 1;
const unsigned int ENGINE_MINOR = 1;
const unsigned int ENGINE_HOTFIX = 1;

Vulkan::Vulkan() {
  this->instance = nullptr;
}

Vulkan::~Vulkan() {
  if (instance) {
    Logger::Infof("shutting down Vulkan.");
    vkDestroyInstance(instance, nullptr);
  }
}

bool Vulkan::CheckSupportedExtensions(std::vector<const char*> requiredExtensions) {
  // TODO: How does it know which device? or is it really from the driver only?

  uint32_t extensionCount = 0;
  if (vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr) != VK_SUCCESS) {
    throw Logger::Errorf("vkEnumerateInstanceExtensionProperties() failed.");
  }
  std::vector<VkExtensionProperties> extensions(extensionCount);
  if (vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data()) !=
      VK_SUCCESS) {
    throw Logger::Errorf(
        "vkEnumerateInstanceExtensionProperties() failed. extensionCount: %d",
        extensionCount);
  }

  bool allRequiredSupported = true;
  Logger::Debugf("device extensions:");
  for (const auto& extension : extensions) {
    Logger::Debugf("  reported %s v%d", extension.extensionName, extension.specVersion);
  }
  bool found;
  for (const auto& required : requiredExtensions) {
    found = false;
    for (const auto& extension : extensions) {
      if (strcmp(required, extension.extensionName) == 0) {
        found = true;
        break;
      }
    }
    if (!found) {
      allRequiredSupported = false;
      Logger::Debugf("  missing %s", required);
    }
  }
  return allRequiredSupported;
}

void Vulkan::CreateInstance(
    std::unique_ptr<VkApplicationInfo> appInfo, std::vector<const char*> extensionNames) {
  // https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkInstanceCreateInfo.html
  auto createInfo = std::make_unique<VkInstanceCreateInfo>();

  // the type of this structure.
  createInfo->sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;

  // NULL or a pointer to a structure extending this structure.
  createInfo->pNext = nullptr;  // default

  // a bitmask of VkInstanceCreateFlagBits indicating the behavior of the instance.
  // https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkInstanceCreateFlagBits.html
  createInfo->flags = 0x0;  // default

  // NULL or a pointer to a VkApplicationInfo structure. If not NULL, this information helps
  // implementations recognize behavior inherent to classes of applications.
  createInfo->pApplicationInfo = appInfo.get();

  // the number of global layers to enable.
  // createInfo->enabledLayerCount = 0;

  // a pointer to an array of enabledLayerCount null-terminated UTF-8 strings containing the
  // names of layers to enable for the created instance. The layers are loaded in the order they are
  // listed in this array, with the first array element being the closest to the application, and
  // the last array element being the closest to the driver. See the
  // https://registry.khronos.org/vulkan/specs/1.3-extensions/html/vkspec.html#extendingvulkan-layers
  // section for further details.
  // createInfo->ppEnabledLayerNames = nullptr;

  // the number of global extensions to enable
  createInfo->enabledExtensionCount = extensionNames.size();

  // a pointer to an array of enabledExtensionCount null-terminated UTF-8 strings containing the
  // names of extensions to enable.
  createInfo->ppEnabledExtensionNames = extensionNames.data();

  // Create a new Vulkan instance
  // https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/vkCreateInstance.html
  if (vkCreateInstance(createInfo.get(), nullptr, &this->instance) != VK_SUCCESS) {
    throw mks::Logger::Errorf("Failed to create Vulkan instance.");
  }
}

std::unique_ptr<VkApplicationInfo> Vulkan::DescribeApplication(
    const char* name,
    const unsigned int major,
    const unsigned int minor,
    const unsigned int hotfix) {
  // https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkApplicationInfo.html
  auto appInfo = std::make_unique<VkApplicationInfo>();

  // the type of this structure.
  appInfo->sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;

  // NULL or a pointer to a structure extending this structure.
  // ie. optional extension information
  appInfo->pNext = nullptr;  // default

  // NULL or is a pointer to a null-terminated UTF-8 string containing the name of the
  // application.
  appInfo->pApplicationName = name;

  // an unsigned integer variable containing the developer-supplied version number of the
  // application.
  appInfo->applicationVersion = VK_MAKE_VERSION(major, minor, hotfix);

  // NULL or is a pointer to a null-terminated UTF-8 string containing the name of the engine (if
  // any) used to create the application.
  appInfo->pEngineName = ENGINE_NAME;

  // an unsigned integer variable containing the developer-supplied version number of the engine
  // used to create the application.
  appInfo->engineVersion = VK_MAKE_VERSION(ENGINE_MAJOR, ENGINE_MINOR, ENGINE_HOTFIX);

  // must be the highest version of Vulkan that the application is designed to use, encoded as
  // described in
  // https://registry.khronos.org/vulkan/specs/1.3-extensions/html/vkspec.html#extendingvulkan-coreversions-versionnumbers.
  // The patch version number specified in apiVersion is ignored when creating an instance object.
  // Only the major and minor versions of the instance must match those requested in apiVersion.
  appInfo->apiVersion = VK_API_VERSION_1_3;

  return appInfo;
}

}  // namespace mks