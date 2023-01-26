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
}

Vulkan::~Vulkan() {
  Logger::Infof("shutting down Vulkan.");
  if (logicalDevice) {
    vkDestroyDevice(logicalDevice, nullptr);
  }
  if (instance) {
    // NOTICE: physicalDevice is destroyed implicitly with instance.
    vkDestroyInstance(instance, nullptr);
  }
}

const bool Vulkan::CheckLayers(std::vector<const char*> requiredLayers) {
  uint32_t layerCount;
  if (vkEnumerateInstanceLayerProperties(&layerCount, nullptr) != VK_SUCCESS) {
    throw Logger::Errorf("vkEnumerateInstanceLayerProperties() failed.");
  }

  std::vector<VkLayerProperties> availableLayers(layerCount);
  if (vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data()) != VK_SUCCESS) {
    throw Logger::Errorf("vkEnumerateInstanceLayerProperties() failed. layerCount: %d", layerCount);
  }

  bool allRequiredSupported = true;
  bool found;
  Logger::Debugf("validation layers:");
  for (const auto& layer : availableLayers) {
    found = false;
    for (const auto& required : requiredLayers) {
      if (strcmp(required, layer.layerName) == 0) {
        found = true;
        break;
      }
    }
    Logger::Debugf(
        "  %s%s",
        layer.layerName,
        // layer.specVersion,
        found ? " (required)" : "");
  }
  for (const auto& required : requiredLayers) {
    found = false;
    for (const auto& layer : availableLayers) {
      if (strcmp(required, layer.layerName) == 0) {
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

const bool Vulkan::CheckExtensions(std::vector<const char*> requiredExtensions) {
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
  bool found;
  for (const auto& extension : extensions) {
    found = false;
    for (const auto& required : requiredExtensions) {
      if (strcmp(required, extension.extensionName) == 0) {
        found = true;
        break;
      }
    }
    Logger::Debugf(
        "  %s%s",
        extension.extensionName,
        // extension.specVersion,
        found ? " (required)" : "");
  }
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

void Vulkan::CreateInstance(
    std::unique_ptr<VkApplicationInfo> appInfo,
    std::vector<const char*> requiredValidationLayers,
    std::vector<const char*> requiredExtensionNames) {
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

#ifdef DEBUG_VULKAN
  // the number of global layers to enable.
  createInfo->enabledLayerCount = requiredValidationLayers.size();

  // a pointer to an array of enabledLayerCount null-terminated UTF-8 strings containing the
  // names of layers to enable for the created instance. The layers are loaded in the order they
  // are listed in this array, with the first array element being the closest to the application,
  // and the last array element being the closest to the driver. See the
  // https://registry.khronos.org/vulkan/specs/1.3-extensions/html/vkspec.html#extendingvulkan-layers
  // section for further details.
  createInfo->ppEnabledLayerNames = requiredValidationLayers.data();
#else
  createInfo->enabledLayerCount = 0;          // default
  createInfo->ppEnabledLayerNames = nullptr;  // default
#endif

  // the number of global extensions to enable
  createInfo->enabledExtensionCount = requiredExtensionNames.size();

  // a pointer to an array of enabledExtensionCount null-terminated UTF-8 strings containing the
  // names of extensions to enable.
  createInfo->ppEnabledExtensionNames = requiredExtensionNames.data();

  // Create a new Vulkan instance
  // https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/vkCreateInstance.html
  if (vkCreateInstance(createInfo.get(), nullptr, &this->instance) != VK_SUCCESS) {
    throw mks::Logger::Errorf("Failed to create Vulkan instance.");
  }
}

const bool Vulkan::UseDevice(const int requiredDeviceIndex) {
  uint32_t deviceCount = 0;
  if (vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr) != VK_SUCCESS) {
    throw Logger::Errorf("vkEnumeratePhysicalDevices() failed.");
  }
  if (deviceCount == 0) {
    throw Logger::Errorf("Failed to locate GPU device with Vulkan support.");
  }
  std::vector<VkPhysicalDevice> devices(deviceCount);
  if (vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data()) != VK_SUCCESS) {
    throw Logger::Errorf("vkEnumeratePhysicalDevices() failed. deviceCount: %d", deviceCount);
  }

  Logger::Debugf("devices:");
  for (unsigned int i = 0; i < devices.size(); i++) {
    const auto& device = devices[i];
    VkPhysicalDeviceProperties props;
    vkGetPhysicalDeviceProperties(device, &props);

    Logger::Debugf(
        "  %u: %s%s",
        i,
        props.deviceName,
        i == requiredDeviceIndex ? " (selected)" : "");
  }

  if (requiredDeviceIndex < devices.size()) {
    physicalDevice = devices[requiredDeviceIndex];
    return true;
  }
  Logger::Debugf("  missing device index %d", requiredDeviceIndex);
  return false;
}

void Vulkan::CheckQueues() const {
  uint32_t queueFamilyCount = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);

  std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
  vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());

  Logger::Debugf("device queue families:");
  for (unsigned int i = 0; i < queueFamilies.size(); i++) {
    const auto& queueFamily = queueFamilies[i];
    Logger::Debugf(
        "  %u: flags:%s%s%s%s%s%s",
        i,
        (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) ? " GRAPHICS" : "",
        (queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT) ? " COMPUTE" : "",
        (queueFamily.queueFlags & VK_QUEUE_TRANSFER_BIT) ? " TRANSFER" : "",
        (queueFamily.queueFlags & VK_QUEUE_SPARSE_BINDING_BIT) ? " SPARSE_BINDING" : "",
        (queueFamily.queueFlags & VK_QUEUE_PROTECTED_BIT) ? " PROTECTED" : "",
        (queueFamily.queueFlags & VK_QUEUE_OPTICAL_FLOW_BIT_NV) ? " OPTICAL_FLOW" : "");
    // if VK_KHR_video_decode_queue extension:
    // VK_QUEUE_VIDEO_DECODE_BIT_KHR
    // ifdef VK_ENABLE_BETA_EXTENSIONS:
    // VK_QUEUE_VIDEO_ENCODE_BIT_KHR
  }

  // TODO: add logic to validate which/whether device+queue combinations we want to use
}

}  // namespace mks