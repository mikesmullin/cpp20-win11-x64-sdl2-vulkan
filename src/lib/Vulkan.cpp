#include "Vulkan.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <algorithm>
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

const bool Vulkan::CheckInstanceLayers(const std::vector<const char*> requiredLayers) {
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
  if (layerCount == 0) {
    throw Logger::Errorf("Failed to locate instance validation layer support.");
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

const bool Vulkan::CheckInstanceExtensions(const std::vector<const char*> requiredExtensions) {
  // list the extensions supported by this driver
  // https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/vkEnumerateInstanceExtensionProperties.html
  uint32_t extensionCount = 0;
  if (vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr) != VK_SUCCESS) {
    throw Logger::Errorf("vkEnumerateInstanceExtensionProperties() failed.");
  }
  if (extensionCount == 0) {
    throw Logger::Errorf("Failed to locate instance extension support.");
  }
  std::vector<VkExtensionProperties> supportedExtensions(extensionCount);
  // Returns up to requested number of global extension properties
  if (vkEnumerateInstanceExtensionProperties(
          nullptr,
          &extensionCount,
          supportedExtensions.data()) != VK_SUCCESS) {
    throw Logger::Errorf(
        "vkEnumerateInstanceExtensionProperties() failed. extensionCount: %d",
        extensionCount);
  }

  // debug: print list of extensions to console
  bool allRequiredSupported = true;
  Logger::Debugf("instance extensions:");
  bool found;
  for (const auto& extension : supportedExtensions) {
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
    for (const auto& extension : supportedExtensions) {
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
  if (instance) {
    if (logicalDevice) {
      if (swapChain) {
        for (auto imageView : swapChainImageViews) {
          vkDestroyImageView(logicalDevice, imageView, nullptr);
        }
        vkDestroySwapchainKHR(logicalDevice, swapChain, nullptr);
      }
      vkDestroyDevice(logicalDevice, nullptr);
      if (surface) {
        vkDestroySurfaceKHR(instance, surface, nullptr);
      }
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

void Vulkan::LocateQueueFamilies() {
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
  bool same = false;
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

    // strategy: select fewest family indices where required queues are present
    if (!same) {
      if (graphics && present) {
        // prioritize any queue with both
        same = true;
        pdqs.graphics.index = i;
        pdqs.present.index = i;
      } else if (graphics) {
        pdqs.graphics.index = i;
      } else if (present) {
        pdqs.present.index = i;
      }
    }

    Logger::Debugf(
        "  %u: flags:%s%s%s%s%s%s%s",
        i,
        present ? " PRESENT" : "",
        graphics ? " GRAPHICS" : "",
        compute ? " COMPUTE" : "",
        transfer ? " TRANSFER" : "",
        sparse ? " SPARSE" : "",
        protect ? " PROTECT" : "",
        optical ? " OPTICAL" : "");
  }

  Logger::Debugf("  selected: graphics: %u, present: %u", pdqs.graphics.index, pdqs.present.index);
}

const bool Vulkan::CheckPhysicalDeviceExtensions(
    const std::vector<const char*> requiredExtensions) const {
  // list the extensions supported by this physical device
  uint32_t extensionCount;
  if (vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, nullptr) !=
      VK_SUCCESS) {
    throw Logger::Errorf("vkEnumerateDeviceExtensionProperties() failed.");
  }

  std::vector<VkExtensionProperties> supportedExtensions(extensionCount);
  if (vkEnumerateDeviceExtensionProperties(
          physicalDevice,
          nullptr,
          &extensionCount,
          supportedExtensions.data()) != VK_SUCCESS) {
    throw Logger::Errorf(
        "vkEnumerateDeviceExtensionProperties() failed. extensionCount: %d",
        extensionCount);
  }

  // debug: print list of extensions to console
  bool allRequiredSupported = true;
  Logger::Debugf("required device extensions:");
  // validate the required extensions are all found
  bool found;
  for (const auto& required : requiredExtensions) {
    found = false;
    for (const auto& extension : supportedExtensions) {
      if (strcmp(required, extension.extensionName) == 0) {
        found = true;
        break;
      }
    }
    if (!found) {
      allRequiredSupported = false;
    }
    Logger::Debugf("  %s%s", required, found ? " (required)" : " (missing)");
  }
  return allRequiredSupported;
}

const bool Vulkan::CheckSwapChainSupport() {
  const std::vector<const char*> requiredPhysicalDeviceExtensions = {
      VK_KHR_SWAPCHAIN_EXTENSION_NAME};
  const bool extensionsSupported = CheckPhysicalDeviceExtensions(requiredPhysicalDeviceExtensions);
  if (!extensionsSupported) {
    throw Logger::Errorf("Missing VK_KHR_swapchain extension on physical device.");
  }

  if (vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
          physicalDevice,
          surface,
          &swapChainSupport.capabilities) != VK_SUCCESS) {
    throw Logger::Errorf("vkGetPhysicalDeviceSurfaceCapabilitiesKHR() failed.");
  }

  uint32_t formatCount;
  if (vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, nullptr) !=
      VK_SUCCESS) {
    throw Logger::Errorf("vkGetPhysicalDeviceSurfaceFormatsKHR() failed.");
  }
  if (formatCount != 0) {
    swapChainSupport.formats.resize(formatCount);
    if (vkGetPhysicalDeviceSurfaceFormatsKHR(
            physicalDevice,
            surface,
            &formatCount,
            swapChainSupport.formats.data()) != VK_SUCCESS) {
      throw Logger::Errorf(
          "vkGetPhysicalDeviceSurfaceFormatsKHR() failed. formatCount: %u",
          formatCount);
    }
  }

  uint32_t presentModeCount;
  if (vkGetPhysicalDeviceSurfacePresentModesKHR(
          physicalDevice,
          surface,
          &presentModeCount,
          nullptr) != VK_SUCCESS) {
    throw Logger::Errorf("vkGetPhysicalDeviceSurfacePresentModesKHR() failed.");
  }
  if (presentModeCount != 0) {
    swapChainSupport.presentModes.resize(presentModeCount);
    if (vkGetPhysicalDeviceSurfacePresentModesKHR(
            physicalDevice,
            surface,
            &presentModeCount,
            swapChainSupport.presentModes.data()) != VK_SUCCESS) {
      throw Logger::Errorf(
          "vkGetPhysicalDeviceSurfacePresentModesKHR() failed. presentModeCount: %u",
          presentModeCount);
    }
  }

  return extensionsSupported && !swapChainSupport.formats.empty() &&
         !swapChainSupport.presentModes.empty();
}

void Vulkan::UseLogicalDevice(
    const std::vector<const char*> requiredValidationLayers,
    std::vector<const char*> requiredPhysicalDeviceExtensions) {
  if (VK_NULL_HANDLE == physicalDevice) {
    throw Logger::Errorf("physicalDevice is null.");
  }

  // enumerate the queue families on current physical device
  LocateQueueFamilies();

  // for each unique queue family index,
  // construct a request for pointer to its VkQueue
  if (!pdqs.graphics.index.has_value()) {
    throw Logger::Errorf("GRAPHICS queue not found within queue families of physical device.");
  }
  if (!pdqs.present.index.has_value()) {
    throw Logger::Errorf("PRESENT queue not found within queue families of physical device.");
  }
  const bool same = pdqs.graphics.index.value() == pdqs.present.index.value();
  std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
  {
    // Structure specifying parameters of a newly created device queue
    // https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkDeviceQueueCreateInfo.html
    VkDeviceQueueCreateInfo createInfo{};

    // the type of this structure.
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;

    // NULL or a pointer to a structure extending this structure.
    // createInfo.pNext = NULL;

    // a bitmask indicating behavior of the queues.
    // createInfo.flags = 0;

    // an unsigned integer indicating the index of the queue family in which to create the queues on
    // this device. This index corresponds to the index of an element of the pQueueFamilyProperties
    // array, and that was returned by vkGetPhysicalDeviceQueueFamilyProperties.
    createInfo.queueFamilyIndex = pdqs.graphics.index.value();

    // an unsigned integer specifying the number of queues to create in the queue family
    // indicated by queueFamilyIndex, and with the behavior specified by flags.
    createInfo.queueCount = 1;

    // a pointer to an array of queueCount normalized floating point values, specifying priorities
    // of work that will be submitted to each created queue.
    const float queuePriority = 1.0f;
    createInfo.pQueuePriorities = &queuePriority;

    // one request per unique queue family index
    queueCreateInfos.push_back(createInfo);
  }
  if (!same) {
    VkDeviceQueueCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    createInfo.queueFamilyIndex = pdqs.present.index.value();
    createInfo.queueCount = 1;
    const float queuePriority = 1.0f;
    createInfo.pQueuePriorities = &queuePriority;
    queueCreateInfos.push_back(createInfo);
  }

  // Structure describing the fine-grained features that can be supported by an implementation
  // https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkPhysicalDeviceFeatures.html
  VkPhysicalDeviceFeatures deviceFeatures{};

  // Structure specifying parameters of a newly created [logical] device
  // https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkDeviceCreateInfo.html
  VkDeviceCreateInfo createInfo{};
  // the type of this structure.
  createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

  // NULL or a pointer to a structure extending this structure.
  createInfo.pNext = NULL;

  // reserved for future use.
  createInfo.flags = static_cast<uint32_t>(0);

  // unsigned integer size of the pQueueCreateInfos array.
  createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());

  // pointer to an array of VkDeviceQueueCreateInfo structures describing the queues that are
  // requested to be created along with the logical device.
  createInfo.pQueueCreateInfos = queueCreateInfos.data();

// define validation layers to be used, per-device.
// deprecated, because we also defined it on the VkInstance,
// but recommended to also keep here for backward-compatibility.
#ifdef DEBUG_VULKAN
  createInfo.enabledLayerCount = static_cast<uint32_t>(requiredValidationLayers.size());
  createInfo.ppEnabledLayerNames = requiredValidationLayers.data();
#else
  createInfo.enabledLayerCount = 0;
#endif

  // number of device extensions to enable.
  createInfo.enabledExtensionCount = static_cast<uint32_t>(requiredPhysicalDeviceExtensions.size());

  // pointer to an array of enabledExtensionCount null-terminated UTF-8 strings containing the names
  // of extensions to enable for the created device.
  createInfo.ppEnabledExtensionNames = requiredPhysicalDeviceExtensions.data();

  // NULL or a pointer to a VkPhysicalDeviceFeatures structure containing boolean indicators of all
  // the features to be enabled.
  createInfo.pEnabledFeatures = &deviceFeatures;

  // Create a new [logical] device instance
  // https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/vkCreateDevice.html
  if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &logicalDevice) != VK_SUCCESS) {
    throw Logger::Errorf("vkCreateDevice() failed.");
  }

  // https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/vkGetDeviceQueue.html
  vkGetDeviceQueue(logicalDevice, pdqs.graphics.index.value(), 0, &pdqs.graphics.queue);
  if (!same) {
    vkGetDeviceQueue(logicalDevice, pdqs.present.index.value(), 0, &pdqs.present.queue);
  }
}

void Vulkan::CreateSwapChain() {
  bool found1 = false;
  VkSurfaceFormatKHR format;
  for (const auto& availableFormat : swapChainSupport.formats) {
    if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
        availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
      format = availableFormat;
      found1 = true;
      break;
    }
  }
  if (!found1) {
    throw Logger::Errorf(
        "Couldn't locate physical device support for the swap chain format we wanted. format: "
        "VK_FORMAT_B8G8R8A8_SRGB, colorSpace: VK_COLOR_SPACE_SRGB_NONLINEAR_KHR");
  }

  bool found2 = false;
  VkPresentModeKHR mode;
  for (const auto& availablePresentMode : swapChainSupport.presentModes) {
    if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
      mode = availablePresentMode;
      found2 = true;
      break;
    }
  }
  if (!found2) {
    mode = VK_PRESENT_MODE_FIFO_KHR;
  }

  VkExtent2D extent;
  if (swapChainSupport.capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
    extent = swapChainSupport.capabilities.currentExtent;
  } else {
    extent.width = std::clamp(
        width,
        swapChainSupport.capabilities.minImageExtent.width,
        swapChainSupport.capabilities.maxImageExtent.width);
    extent.height = std::clamp(
        height,
        swapChainSupport.capabilities.minImageExtent.height,
        swapChainSupport.capabilities.maxImageExtent.height);
  }

  const uint32_t imageCount = std::clamp(
      swapChainSupport.capabilities.minImageCount + 1,
      swapChainSupport.capabilities.minImageCount,
      swapChainSupport.capabilities.maxImageCount);

  VkSwapchainCreateInfoKHR createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  createInfo.surface = surface;
  createInfo.minImageCount = imageCount;
  createInfo.imageFormat = format.format;
  createInfo.imageColorSpace = format.colorSpace;
  createInfo.imageExtent = extent;
  createInfo.imageArrayLayers = 1;
  createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
  if (pdqs.graphics.index == pdqs.present.index) {
    createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    createInfo.queueFamilyIndexCount = 0;      // default
    createInfo.pQueueFamilyIndices = nullptr;  // default
  } else {
    createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
    createInfo.queueFamilyIndexCount = 2;
    const std::vector<uint32_t> indicies = {
        pdqs.graphics.index.value(),
        pdqs.present.index.value(),
    };
    createInfo.pQueueFamilyIndices = indicies.data();
  }
  createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
  createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
  createInfo.presentMode = mode;
  createInfo.clipped = VK_TRUE;
  // TODO: accept this as param
  createInfo.oldSwapchain = VK_NULL_HANDLE;

  if (vkCreateSwapchainKHR(logicalDevice, &createInfo, nullptr, &swapChain) != VK_SUCCESS) {
    throw Logger::Errorf("vkCreateSwapchainKHR failed.");
  }

  uint32_t receivedImageCount;
  if (vkGetSwapchainImagesKHR(logicalDevice, swapChain, &receivedImageCount, nullptr) !=
      VK_SUCCESS) {
    throw Logger::Errorf("vkGetSwapchainImagesKHR() failed");
  }
  if (receivedImageCount < imageCount) {
    throw Logger::Errorf(
        "Mismatch in swap chain image count. requested: %u, received: %u",
        imageCount,
        receivedImageCount);
  }
  swapChainImages.resize(receivedImageCount);
  if (vkGetSwapchainImagesKHR(
          logicalDevice,
          swapChain,
          &receivedImageCount,
          swapChainImages.data()) != VK_SUCCESS) {
    throw Logger::Errorf("vkGetSwapchainImagesKHR() failed. imageCount: %u", receivedImageCount);
  }

  swapChainImageFormat = format.format;
  swapChainExtent = extent;

  Logger::Debugf("swap chain:");
  Logger::Debugf(
      "  width: %u, height: %u, imageCount: %u",
      extent.width,
      extent.height,
      receivedImageCount);

  swapChainImageViews.resize(swapChainImages.size());
  for (size_t i = 0; i < swapChainImages.size(); i++) {
    VkImageViewCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    createInfo.image = swapChainImages[i];
    createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    createInfo.format = swapChainImageFormat;
    createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    createInfo.subresourceRange.baseMipLevel = 0;
    createInfo.subresourceRange.levelCount = 1;
    createInfo.subresourceRange.baseArrayLayer = 0;
    createInfo.subresourceRange.layerCount = 1;
    if (vkCreateImageView(logicalDevice, &createInfo, nullptr, &swapChainImageViews[i]) !=
        VK_SUCCESS) {
      throw Logger::Errorf("vkCreateImageView failed. i: %u", i);
    }
  }
}

}  // namespace mks