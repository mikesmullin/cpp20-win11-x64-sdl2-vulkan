#include "Vulkan.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <memory>
#include <set>
#include <vector>

#include "Logger.hpp"

// TODO: finish
// https://vulkan-tutorial.com/Drawing_a_triangle/Setup/Instance
// https://wiki.libsdl.org/SDL2/SDL_Vulkan_GetInstanceExtensions

namespace mks {

const bool Vulkan::CheckLayers(const std::vector<const char*> requiredLayers) {
  // BACKGROUND: There were formerly two different types of validation layers in Vulkan: instance
  // and device specific. The idea was that instance layers would only check calls related to global
  // Vulkan objects like instances, and device specific layers would only check calls related to a
  // specific GPU. Device specific layers have now been deprecated, which means that instance
  // validation layers apply to all Vulkan calls.

  // list which validation layers are supported by driver
  // https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/vkEnumerateInstanceLayerProperties.html
  uint32_t layerCount;

  if (vkEnumerateInstanceLayerProperties(&layerCount, nullptr) != VK_SUCCESS) {
    throw Logger::Errorf("vkEnumerateInstanceLayerProperties() failed.");
  }
  std::vector<VkLayerProperties> availableLayers(layerCount);
  // Returns up to requested number of global layer properties
  if (vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data()) != VK_SUCCESS) {
    throw Logger::Errorf("vkEnumerateInstanceLayerProperties() failed. layerCount: %d", layerCount);
  }

  // debug: print list of validation layers to console
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
    Logger::Debugf("  %s%s", layer.layerName, found ? " (required)" : "");
  }
  // validate the required validation layers are all found
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

const bool Vulkan::CheckExtensions(const std::vector<const char*> requiredExtensions) {
  // list the extensions supported by this driver
  // https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/vkEnumerateInstanceExtensionProperties.html
  uint32_t extensionCount = 0;
  if (vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr) != VK_SUCCESS) {
    throw Logger::Errorf("vkEnumerateInstanceExtensionProperties() failed.");
  }
  std::vector<VkExtensionProperties> extensions(extensionCount);
  // Returns up to requested number of global extension properties
  if (vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data()) !=
      VK_SUCCESS) {
    throw Logger::Errorf(
        "vkEnumerateInstanceExtensionProperties() failed. extensionCount: %d",
        extensionCount);
  }

  // debug: print list of extensions to console
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
    Logger::Debugf("  %s%s", extension.extensionName, found ? " (required)" : "");
  }

  // validate the required extensions are all found
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

Vulkan::Vulkan(
    const char* name,
    const unsigned int major,
    const unsigned int minor,
    const unsigned int hotfix,
    const std::vector<const char*> requiredValidationLayers,
    const std::vector<const char*> requiredExtensionNames) {
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
  appInfo->pEngineName = "MKS";

  // an unsigned integer variable containing the developer-supplied version number of the engine
  // used to create the application.
  appInfo->engineVersion = VK_MAKE_VERSION(1, 0, 0);

  // must be the highest version of Vulkan that the application is designed to use, encoded as
  // described in
  // https://registry.khronos.org/vulkan/specs/1.3-extensions/html/vkspec.html#extendingvulkan-coreversions-versionnumbers.
  // The patch version number specified in apiVersion is ignored when creating an instance object.
  // Only the major and minor versions of the instance must match those requested in apiVersion.
  appInfo->apiVersion = VK_API_VERSION_1_3;

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
  if (vkCreateInstance(createInfo.get(), nullptr, &instance) != VK_SUCCESS) {
    throw mks::Logger::Errorf("Failed to create Vulkan instance.");
  }
}

Vulkan::~Vulkan() {
  Logger::Infof("shutting down Vulkan.");
  if (logicalDevice) {
    vkDestroyDevice(logicalDevice, nullptr);
  }
  if (instance) {
    if (surface) {
      vkDestroySurfaceKHR(instance, surface, nullptr);
    }
    // NOTICE: physicalDevice is destroyed implicitly with instance.
    vkDestroyInstance(instance, nullptr);
  }
}

const bool Vulkan::UsePhysicalDevice(const unsigned int requiredDeviceIndex) {
  // list GPUs
  uint32_t deviceCount = 0;
  // https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/vkEnumeratePhysicalDevices.html
  if (vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr) != VK_SUCCESS) {
    throw Logger::Errorf("vkEnumeratePhysicalDevices() failed.");
  }
  if (deviceCount == 0) {
    throw Logger::Errorf("Failed to locate GPU device with Vulkan support.");
  }
  std::vector<VkPhysicalDevice> devices(deviceCount);
  // Enumerates the physical devices accessible to a Vulkan instance
  if (vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data()) != VK_SUCCESS) {
    throw Logger::Errorf("vkEnumeratePhysicalDevices() failed. deviceCount: %d", deviceCount);
  }

  // debug: print all GPUs found
  Logger::Debugf("devices:");
  for (unsigned int i = 0; i < devices.size(); i++) {
    const auto& device = devices[i];
    // Structure specifying physical device properties
    // https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkPhysicalDeviceProperties.html
    VkPhysicalDeviceProperties deviceProperties;
    // Returns properties of a physical device
    // https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/vkGetPhysicalDeviceProperties.html
    vkGetPhysicalDeviceProperties(device, &deviceProperties);

    // Structure describing the fine-grained features that can be supported by an implementation
    // https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkPhysicalDeviceFeatures.html
    VkPhysicalDeviceFeatures deviceFeatures;
    // Reports capabilities of a physical device
    // https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/vkGetPhysicalDeviceFeatures.html
    vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

    // assumption: we only want the type of GPU device used to render video games
    const bool discrete = deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
    const bool geometry = deviceFeatures.geometryShader;

    // debug: list each GPU device found
    Logger::Debugf(
        "  %u: %s%s%s%s",
        i,
        deviceProperties.deviceName,
        i == requiredDeviceIndex ? " (selected)" : "",
        discrete ? " DISCRETE" : "",
        geometry ? " GEOMETRY_SHADER" : "");

    // select one GPU to be the active/default/current for all subsequent Vulkan methods;
    // it must meet certain minimum requirements
    if (i == requiredDeviceIndex && discrete && geometry) {
      physicalDevice = device;
      return true;
    }
  }
  Logger::Debugf("  missing device index %d", requiredDeviceIndex);
  return false;
}

const bool Vulkan::CheckQueues() {
  // validate state
  if (VK_NULL_HANDLE == physicalDevice) {
    throw Logger::Errorf("physicalDevice is null.");
  }
  if (!surface) {
    throw Logger::Errorf("surface is null.");
  }

  uint32_t queueFamilyCount = 0;
  // https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/vkGetPhysicalDeviceQueueFamilyProperties.html
  vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);

  std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
  // Reports properties of the queues of the specified physical device
  vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());

  // debug: list all queue families found on the current physical device
  Logger::Debugf("device queue families:");
  for (uint32_t i = 0; i < queueFamilies.size(); i++) {
    const auto& queueFamily = queueFamilies[i];

    const bool graphics = queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT;
    const bool compute = queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT;
    const bool transfer = queueFamily.queueFlags & VK_QUEUE_TRANSFER_BIT;
    const bool sparse = queueFamily.queueFlags & VK_QUEUE_SPARSE_BINDING_BIT;
    const bool protect = queueFamily.queueFlags & VK_QUEUE_PROTECTED_BIT;
    // if VK_KHR_video_decode_queue extension:
    // const bool video_decode = queueFamily.queueFlags & VK_QUEUE_VIDEO_DECODE_BIT;
    // ifdef VK_ENABLE_BETA_EXTENSIONS:
    // const bool video_encode = queueFamily.queueFlags & VK_QUEUE_VIDEO_ENCODE_BIT;
    const bool optical = queueFamily.queueFlags & VK_QUEUE_OPTICAL_FLOW_BIT_NV;

    VkBool32 present = false;
    // Query if presentation is supported
    // https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/vkGetPhysicalDeviceSurfaceSupportKHR.html
    if (vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface, &present) != VK_SUCCESS) {
      Logger::Debugf("vkGetPhysicalDeviceSurfaceSupportKHR() failed. queueFamilyIndex: %u", i);
    }

    if (graphics) {
      pdqfs.graphics.supported = true;
      if (!pdqfs.graphics.selectedIndex) {
        pdqfs.graphics.selectedIndex = i;
      }
      pdqfs.graphics.supportedIndices.push_back(i);
    }
    if (compute) {
      pdqfs.compute.supported = true;
      if (!pdqfs.compute.selectedIndex) {
        pdqfs.compute.selectedIndex = i;
      }
      pdqfs.compute.supportedIndices.push_back(i);
    }
    if (transfer) {
      pdqfs.transfer.supported = true;
      if (!pdqfs.transfer.selectedIndex) {
        pdqfs.transfer.selectedIndex = i;
      }
      pdqfs.transfer.supportedIndices.push_back(i);
    }
    if (sparse) {
      pdqfs.sparse.supported = true;
      if (!pdqfs.sparse.selectedIndex) {
        pdqfs.sparse.selectedIndex = i;
      }
      pdqfs.sparse.supportedIndices.push_back(i);
    }
    if (protect) {
      pdqfs.protect.supported = true;
      if (!pdqfs.protect.selectedIndex) {
        pdqfs.protect.selectedIndex = i;
      }
      pdqfs.protect.supportedIndices.push_back(i);
    }
    if (optical) {
      pdqfs.optical.supported = true;
      if (!pdqfs.optical.selectedIndex) {
        pdqfs.optical.selectedIndex = i;
      }
      pdqfs.optical.supportedIndices.push_back(i);
    }
    if (present) {
      pdqfs.present.supported = true;
      if (!pdqfs.present.selectedIndex) {
        pdqfs.present.selectedIndex = i;
      }
      pdqfs.present.supportedIndices.push_back(i);
    }

    Logger::Debugf(
        "  %u: flags:%s%s%s%s%s%s%s%s%s",
        i,
        present ? " PRESENT" : "",
        graphics ? " GRAPHICS" : "",
        compute ? " COMPUTE" : "",
        transfer ? " TRANSFER" : "",
        sparse ? " SPARSE" : "",
        protect ? " PROTECTED" : "",
        optical ? " OPTICAL_FLOW" : "",
        i == pdqfs.present.selectedIndex ? " (select present)" : "",
        i == pdqfs.graphics.selectedIndex ? " (select graphics)" : "");
  }

  // uint32_t formatCount;
  // vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, nullptr);
  // if (formatCount != 0) {
  //   details.formats.resize(formatCount);
  //   vkGetPhysicalDeviceSurfaceFormatsKHR(
  //       physicalDevice,
  //       surface,
  //       &formatCount,
  //       details.formats.data());
  // }

  // true if: not any required that aren't supported
  return !(
      (pdqfs.graphics.required && !pdqfs.graphics.supported) ||
      (pdqfs.compute.required && !pdqfs.compute.supported) ||
      (pdqfs.transfer.required && !pdqfs.transfer.supported) ||
      (pdqfs.sparse.required && !pdqfs.sparse.supported) ||
      (pdqfs.protect.required && !pdqfs.protect.supported) ||
      (pdqfs.optical.required && !pdqfs.optical.supported) ||
      (pdqfs.present.required && !pdqfs.present.supported));
}

void Vulkan::UseLogicalDevice(const std::vector<const char*> requiredValidationLayers) {
  if (VK_NULL_HANDLE == physicalDevice) {
    throw Logger::Errorf("physicalDevice is null.");
  }
  if (!pdqfs.graphics.selectedIndex.has_value()) {
    throw Logger::Errorf("graphicsQueueFamilyIndex is null.");
  }
  if (!pdqfs.present.selectedIndex.has_value()) {
    throw Logger::Errorf("presentQueueFamilyIndex is null.");
  }

  std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
  std::set<uint32_t> uniqueQueueFamilies = {
      pdqfs.graphics.selectedIndex.value(),
      pdqfs.present.selectedIndex.value()};
  float queuePriority = 1.0f;
  for (uint32_t queueFamily : uniqueQueueFamilies) {
    VkDeviceQueueCreateInfo queueCreateInfo{};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueFamilyIndex = queueFamily;
    queueCreateInfo.queueCount = 1;
    queueCreateInfo.pQueuePriorities = &queuePriority;
    queueCreateInfos.push_back(queueCreateInfo);
  }

  VkPhysicalDeviceFeatures deviceFeatures{};

  VkDeviceCreateInfo createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
  createInfo.pQueueCreateInfos = queueCreateInfos.data();
  createInfo.pEnabledFeatures = &deviceFeatures;
  createInfo.enabledExtensionCount = 0;

#ifdef DEBUG_VULKAN
  createInfo.enabledLayerCount = static_cast<uint32_t>(requiredValidationLayers.size());
  createInfo.ppEnabledLayerNames = requiredValidationLayers.data();
#else
  createInfo.enabledLayerCount = 0;
#endif

  if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &logicalDevice) != VK_SUCCESS) {
    throw Logger::Errorf("vkCreateDevice failed.");
  }

  vkGetDeviceQueue(logicalDevice, pdqfs.graphics.selectedIndex.value(), 0, &pdqfs.graphics.queue);
  vkGetDeviceQueue(logicalDevice, pdqfs.present.selectedIndex.value(), 0, &pdqfs.present.queue);
}

}  // namespace mks