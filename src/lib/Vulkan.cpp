#include "Vulkan.hpp"

#include <algorithm>
#include <array>
#include <cstdint>
#include <vector>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "Base.hpp"
#include "Logger.hpp"
#include "Shader.hpp"

namespace mks {

std::vector<const char*> Vulkan::requiredValidationLayers{};
std::vector<const char*> Vulkan::requiredDriverExtensionNames{};

Vulkan::Vulkan() {
}

Vulkan::~Vulkan() {
}

void Vulkan::CreateInstance(
    const char* name,
    const unsigned int major,
    const unsigned int minor,
    const unsigned int hotfix) {
  // https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkApplicationInfo.html
  VkApplicationInfo appInfo{};

  // the type of this structure.
  appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;

  // NULL or a pointer to a structure extending this structure.
  // ie. optional extension information
  appInfo.pNext = nullptr;  // default

  // NULL or is a pointer to a null-terminated UTF-8 string containing the name of the
  // application.
  appInfo.pApplicationName = name;

  // an unsigned integer variable containing the developer-supplied version number of the
  // application.
  appInfo.applicationVersion = VK_MAKE_VERSION(major, minor, hotfix);

  // NULL or is a pointer to a null-terminated UTF-8 string containing the name of the engine (if
  // any) used to create the application.
  appInfo.pEngineName = "MKS";

  // an unsigned integer variable containing the developer-supplied version number of the engine
  // used to create the application.
  appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);

  // must be the highest version of Vulkan that the application is designed to use, encoded as
  // described in
  // https://registry.khronos.org/vulkan/specs/1.3-extensions/html/vkspec.html#extendingvulkan-coreversions-versionnumbers.
  // The patch version number specified in apiVersion is ignored when creating an instance object.
  // Only the major and minor versions of the instance must match those requested in apiVersion.
  appInfo.apiVersion = VK_API_VERSION_1_3;

  // https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkInstanceCreateInfo.html
  VkInstanceCreateInfo createInfo{};

  // the type of this structure.
  createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;

  // NULL or a pointer to a structure extending this structure.
  createInfo.pNext = nullptr;  // default

  // a bitmask of VkInstanceCreateFlagBits indicating the behavior of the instance.
  // https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkInstanceCreateFlagBits.html
#if OS_MAC == 1
  createInfo.flags =
      // enable MoltenVK support for MacOS cross-platform support
      VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
#else
  createInfo.flags = 0;  // default
#endif

  // NULL or a pointer to a VkApplicationInfo structure. If not NULL, this information helps
  // implementations recognize behavior inherent to classes of applications.
  createInfo.pApplicationInfo = &appInfo;

#ifdef DEBUG_VULKAN
  // the number of global layers to enable.
  createInfo.enabledLayerCount = requiredValidationLayers.size();

  // a pointer to an array of enabledLayerCount null-terminated UTF-8 strings containing the
  // names of layers to enable for the created instance. The layers are loaded in the order they
  // are listed in this array, with the first array element being the closest to the application,
  // and the last array element being the closest to the driver. See the
  // https://registry.khronos.org/vulkan/specs/1.3-extensions/html/vkspec.html#extendingvulkan-layers
  // section for further details.
  createInfo.ppEnabledLayerNames = requiredValidationLayers.data();
#else
  createInfo.enabledLayerCount = 0;          // default
  createInfo.ppEnabledLayerNames = nullptr;  // default
#endif

  // the number of global extensions to enable
  createInfo.enabledExtensionCount = requiredDriverExtensionNames.size();

  // a pointer to an array of enabledExtensionCount null-terminated UTF-8 strings containing the
  // names of extensions to enable.
  createInfo.ppEnabledExtensionNames = requiredDriverExtensionNames.data();
  // Create a new Vulkan instance
  // https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/vkCreateInstance.html
  if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
    throw Logger::Errorf("Failed to create Vulkan instance.");
  }
}

void Vulkan::InitSwapChain() {
  requiredPhysicalDeviceExtensions.emplace_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
  AssertSwapChainSupport();
  UseLogicalDevice();
  CreateSwapChain();
}

void Vulkan::AssertDriverValidationLayersSupported() {
#ifdef DEBUG_VULKAN
  // SDK-provided layer conveniently bundles all useful standard validation
  requiredValidationLayers.emplace_back("VK_LAYER_KHRONOS_validation");
#endif

  if (requiredValidationLayers.size() > 0) {
    bool supported = CheckInstanceLayers();
    if (!supported) {
      throw Logger::Errorf("Missing required Vulkan validation layers.");
    }

    // TODO: Device-specific layer validation is deprecated. However,
    //   the specification document still recommends that you enable validation layers at device
    //   level as well for compatibility, and it's required by some implementations. we'll see this
    //   code later on:
    //   https://vulkan-tutorial.com/en/Drawing_a_triangle/Setup/Logical_device_and_queues
  }
}

void Vulkan::AssertDriverExtensionsSupported() {
  bool supported = false;
  supported = CheckInstanceExtensions();
  if (!supported) {
    throw Logger::Errorf("Vulkan driver is missing required Vulkan extensions.");
  }
}

const bool Vulkan::CheckInstanceLayers() {
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
    for (const auto& required : requiredValidationLayers) {
      if (strcmp(required, layer.layerName) == 0) {
        found = true;
        break;
      }
    }
    Logger::Debugf("  %s%s", layer.layerName, found ? " (required)" : "");
  }
  // validate the required validation layers are all found
  for (const auto& required : requiredValidationLayers) {
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

const bool Vulkan::CheckInstanceExtensions() {
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
    for (const auto& required : requiredDriverExtensionNames) {
      if (strcmp(required, extension.extensionName) == 0) {
        found = true;
        break;
      }
    }
    Logger::Debugf("  %s%s", extension.extensionName, found ? " (required)" : "");
  }

  // validate the required extensions are all found
  for (const auto& required : requiredDriverExtensionNames) {
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

const void Vulkan::UsePhysicalDevice(const unsigned int requiredDeviceIndex) {
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
    if (i == requiredDeviceIndex /*&& discrete*/ /*&& geometry*/) {
      physicalDevice = device;
      return;
    }
  }
  Logger::Debugf("  missing physical device index %d", requiredDeviceIndex);
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

const bool Vulkan::CheckPhysicalDeviceExtensions() {
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

  // quirk: vulkan spec says if this is supported, we must request it
  const char* special1 = "VK_KHR_portability_subset";

  // print all supported extensions to console
  // Logger::Debugf("supported device extensions:");
  bool special = false;
  for (const auto& extension : supportedExtensions) {
    if (strcmp(special1, extension.extensionName) == 0) {
      requiredPhysicalDeviceExtensions.emplace_back(special1);
      special = true;
    }
    // Logger::Debugf("  %s%s%s", extension.extensionName, special ? " (implicit-required)" : "");
  }

  // debug: print list of extensions to console
  bool allRequiredSupported = true;
  Logger::Debugf("required device extensions:");
  // validate the required extensions are all found
  bool found = false;
  for (const auto& required : requiredPhysicalDeviceExtensions) {
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

const void Vulkan::AssertSwapChainSupport() {
  const bool extensionsSupported = CheckPhysicalDeviceExtensions();
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

  bool supported = extensionsSupported && !swapChainSupport.formats.empty() &&
                   !swapChainSupport.presentModes.empty();
  if (!supported) {
    throw mks::Logger::Errorf("Missing swap chain support on physical device.");
  }
}

void Vulkan::UseLogicalDevice() {
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

  // used with texture images
  deviceFeatures.samplerAnisotropy = VK_TRUE;

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

  // TODO: Need support for macOS retina displays which means the surface resolution will be
  // larger than the window resolution.

  // TODO: these bounds need to be restricted within the maximum capability of the surface and
  // device, whichever is smaller. Unfortunately I was not able to make that work using
  // swapChainSupport.capabilities because (min|max)ImageExtent always returns the current bounds,
  // which is not helpful. In the event that Vulkan notifies us of a surface change, which is not
  // preceded by an SDL resize event. (e.g.,. color-depth change?) Then we are missing a function to
  // determine the new bounds. So we risk a crash here!
  VkExtent2D extent;
  extent.width = bufferWidth;
  extent.height = bufferHeight;

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

  Logger::Debugf(
      "swap chain:\n  width: %u, height: %u, imageCount: %u",
      extent.width,
      extent.height,
      receivedImageCount);
}

void Vulkan::CreateShaderModule(const std::vector<char>& code, VkShaderModule* shaderModule) const {
  VkShaderModuleCreateInfo createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  createInfo.codeSize = code.size();
  createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());
  if (vkCreateShaderModule(logicalDevice, &createInfo, nullptr, shaderModule) != VK_SUCCESS) {
    throw Logger::Errorf("vkCreateShaderModule failed.");
  }
  // TODO: need to keep a global reference to these, and cleanup on shutdown
}

void Vulkan::DestroyShaderModule(const VkShaderModule* shaderModule) const {
  vkDestroyShaderModule(logicalDevice, *shaderModule, nullptr);
}

void Vulkan::CreateImageViews() {
  // size array to fit whatever hardware supports
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

void Vulkan::CreateRenderPass() {
  // single color buffer attachment represented by one of the images from the swap chain
  VkAttachmentDescription colorAttachment{};
  colorAttachment.format = swapChainImageFormat;
  // TODO: we're not doing anything with multisampling yet, so we'll stick to 1 sample.
  colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
  colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  // we render to window handle
  colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

  VkAttachmentReference colorAttachmentRef{};
  colorAttachmentRef.attachment = 0;
  colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

  // Sub-passes are subsequent rendering operations that depend on the contents of framebuffers in
  // previous passes, for example a series of post-processing effects
  VkSubpassDescription subpass{};
  subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
  subpass.colorAttachmentCount = 1;
  subpass.pColorAttachments = &colorAttachmentRef;

  VkSubpassDependency dependency{};
  dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
  dependency.dstSubpass = 0;
  dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  dependency.srcAccessMask = 0;
  dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

  VkRenderPassCreateInfo renderPassInfo{};
  renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
  renderPassInfo.attachmentCount = 1;
  renderPassInfo.pAttachments = &colorAttachment;
  renderPassInfo.subpassCount = 1;
  renderPassInfo.pSubpasses = &subpass;
  renderPassInfo.dependencyCount = 1;
  renderPassInfo.pDependencies = &dependency;

  if (vkCreateRenderPass(logicalDevice, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
    throw Logger::Errorf("vkCreateRenderPass failed.");
  }
}

void Vulkan::CreateDescriptorSetLayout() {
  VkDescriptorSetLayoutBinding uboLayoutBinding{};
  uboLayoutBinding.binding = 0;
  uboLayoutBinding.descriptorCount = 1;
  uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  uboLayoutBinding.pImmutableSamplers = nullptr;
  uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

  VkDescriptorSetLayoutBinding samplerLayoutBinding{};
  samplerLayoutBinding.binding = 1;
  samplerLayoutBinding.descriptorCount = 1;
  samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  samplerLayoutBinding.pImmutableSamplers = nullptr;
  samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

  std::array<VkDescriptorSetLayoutBinding, 2> bindings = {uboLayoutBinding, samplerLayoutBinding};
  VkDescriptorSetLayoutCreateInfo layoutInfo{};
  layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
  layoutInfo.pBindings = bindings.data();

  if (vkCreateDescriptorSetLayout(logicalDevice, &layoutInfo, nullptr, &descriptorSetLayout) !=
      VK_SUCCESS) {
    throw Logger::Errorf("failed to create descriptor set layout!");
  }
}

void Vulkan::CreateGraphicsPipeline(
    const std::string& frag_shader,
    const std::string& vert_shader,
    u32 vertexSize,
    u32 instanceSize,
    u8 attrCount,
    std::vector<u32> bindings,
    std::vector<u32> locations,
    std::vector<u32> formats,
    std::vector<u32> offsets) {
  // NOTICE: pipeline state is immutable; you will make many of these instances

  vertexBuffers.resize(2);  // TODO: hard-code as std::array<,2>
  vertexBufferMemories.resize(2);

  Shader s = {};
  // TODO: Make shaders configurable via input
  auto shader1 = s.readFile(frag_shader);
  auto shader2 = s.readFile(vert_shader);

  VkShaderModule vertShaderModule, fragShaderModule;
  CreateShaderModule(shader1, &fragShaderModule);
  CreateShaderModule(shader2, &vertShaderModule);

  VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
  vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
  vertShaderStageInfo.module = vertShaderModule;
  vertShaderStageInfo.pName = "main";
  // TODO: provide constants at shader stage init via .pSpecializationInfo

  VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
  fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
  fragShaderStageInfo.module = fragShaderModule;
  fragShaderStageInfo.pName = "main";

  VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

  // utilize dynamic viewport and scissor state, so resize can be specified at draw time
  std::vector<VkDynamicState> dynamicStates = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
  VkPipelineDynamicStateCreateInfo dynamicState{};
  dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
  dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
  dynamicState.pDynamicStates = dynamicStates.data();

  std::array<VkVertexInputBindingDescription, 2> bindingDescriptions;
  bindingDescriptions[0].binding = 0;
  bindingDescriptions[0].stride = vertexSize;
  bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
  bindingDescriptions[1].binding = 1;
  bindingDescriptions[1].stride = instanceSize;
  bindingDescriptions[1].inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;

  std::vector<VkVertexInputAttributeDescription> attributeDescriptions(attrCount);
  for (u8 i = 0; i < attrCount; i++) {
    attributeDescriptions[i].binding = bindings[i];
    attributeDescriptions[i].location = locations[i];
    attributeDescriptions[i].format = static_cast<VkFormat>(formats[i]);
    // VK_FORMAT_R32G32_SFLOAT;  // 103
    // VK_FORMAT_R32G32B32_SFLOAT;  // 106
    // VK_FORMAT_R32G32B32A32_SFLOAT;  // 109
    // VK_FORMAT_R32_UINT; // 98
    // VK_FORMAT_R32G32B32_SFLOAT; // 106
    // VK_FORMAT_R32_SFLOAT; // 100
    attributeDescriptions[i].offset = offsets[i];
  }

  VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
  vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
  vertexInputInfo.vertexBindingDescriptionCount = 2;
  vertexInputInfo.pVertexBindingDescriptions = bindingDescriptions.data();
  vertexInputInfo.vertexAttributeDescriptionCount =
      static_cast<uint32_t>(attributeDescriptions.size());
  vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

  // specify what kind of geometry will be drawn from the vertices, and
  // if primitive restart should be enabled.
  VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
  inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
  inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
  inputAssembly.primitiveRestartEnable = VK_FALSE;

  VkPipelineViewportStateCreateInfo viewportState{};
  viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
  viewportState.viewportCount = 1;
  viewportState.scissorCount = 1;

  // Rasterizer
  VkPipelineRasterizationStateCreateInfo rasterizer{};
  rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
  rasterizer.depthClampEnable = VK_FALSE;
  rasterizer.rasterizerDiscardEnable = VK_FALSE;
  rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
  rasterizer.lineWidth = 1.0f;
  rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
  // TODO: which of these do I want to keep?
  // rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
  rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
  rasterizer.depthBiasEnable = VK_FALSE;
  rasterizer.depthBiasConstantFactor = 0.0f;
  rasterizer.depthBiasClamp = 0.0f;
  rasterizer.depthBiasSlopeFactor = 0.0f;

  VkPipelineMultisampleStateCreateInfo multisampling{};
  multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
  multisampling.sampleShadingEnable = VK_FALSE;
  multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
  multisampling.minSampleShading = 1.0f;
  multisampling.pSampleMask = nullptr;
  multisampling.alphaToCoverageEnable = VK_FALSE;
  multisampling.alphaToOneEnable = VK_FALSE;

  // TODO: configure the depth and stencil tests using VkPipelineDepthStencilStateCreateInfo

  VkPipelineColorBlendAttachmentState colorBlendAttachment{};
  colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                                        VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
  colorBlendAttachment.blendEnable = VK_TRUE;
  colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
  colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
  colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
  colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
  colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
  colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

  VkPipelineColorBlendStateCreateInfo colorBlending{};
  colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
  colorBlending.logicOpEnable = VK_FALSE;
  colorBlending.logicOp = VK_LOGIC_OP_COPY;
  colorBlending.attachmentCount = 1;
  colorBlending.pAttachments = &colorBlendAttachment;
  colorBlending.blendConstants[0] = 1.0f;
  colorBlending.blendConstants[1] = 1.0f;
  colorBlending.blendConstants[2] = 1.0f;
  colorBlending.blendConstants[3] = 1.0f;

  VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
  pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  pipelineLayoutInfo.setLayoutCount = 1;
  pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;

  if (vkCreatePipelineLayout(logicalDevice, &pipelineLayoutInfo, nullptr, &pipelineLayout) !=
      VK_SUCCESS) {
    throw Logger::Errorf("vkCreatePipelineLayout failed.");
  }

  VkGraphicsPipelineCreateInfo pipelineInfo{};
  pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
  pipelineInfo.stageCount = 2;
  pipelineInfo.pStages = shaderStages;
  pipelineInfo.pVertexInputState = &vertexInputInfo;
  pipelineInfo.pInputAssemblyState = &inputAssembly;
  pipelineInfo.pViewportState = &viewportState;
  pipelineInfo.pRasterizationState = &rasterizer;
  pipelineInfo.pMultisampleState = &multisampling;
  pipelineInfo.pDepthStencilState = nullptr;
  pipelineInfo.pColorBlendState = &colorBlending;
  pipelineInfo.pDynamicState = &dynamicState;
  pipelineInfo.layout = pipelineLayout;
  pipelineInfo.renderPass = renderPass;
  pipelineInfo.subpass = 0;
  pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
  pipelineInfo.basePipelineIndex = -1;

  if (vkCreateGraphicsPipelines(
          logicalDevice,
          VK_NULL_HANDLE,
          1,
          &pipelineInfo,
          nullptr,
          &graphicsPipeline) != VK_SUCCESS) {
    throw Logger::Errorf("vkCreateGraphicsPipelines failed.");
  }

  DestroyShaderModule(&vertShaderModule);
  DestroyShaderModule(&fragShaderModule);
}

void Vulkan::CreateFrameBuffers() {
  swapChainFramebuffers.resize(swapChainImageViews.size());

  for (size_t i = 0; i < swapChainImageViews.size(); i++) {
    VkImageView attachments[] = {swapChainImageViews[i]};

    VkFramebufferCreateInfo framebufferInfo{};
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.renderPass = renderPass;
    // TODO: couldn't/shouldn't we have one framebuffer, with two attachments?
    framebufferInfo.attachmentCount = 1;
    framebufferInfo.pAttachments = attachments;
    framebufferInfo.width = swapChainExtent.width;
    framebufferInfo.height = swapChainExtent.height;
    framebufferInfo.layers = 1;

    if (vkCreateFramebuffer(logicalDevice, &framebufferInfo, nullptr, &swapChainFramebuffers[i]) !=
        VK_SUCCESS) {
      throw Logger::Errorf("vkCreateFramebuffer failed.");
    }
  }
}

void Vulkan::CreateCommandPool() {
  VkCommandPoolCreateInfo poolInfo{};
  poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
  poolInfo.queueFamilyIndex = pdqs.graphics.index.value();

  if (vkCreateCommandPool(logicalDevice, &poolInfo, nullptr, &commandPool) != VK_SUCCESS) {
    throw Logger::Errorf("vkCreateCommandPool failed.");
  }
}

void Vulkan::CreateCommandBuffers() {
  commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);

  VkCommandBufferAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  allocInfo.commandPool = commandPool;
  allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

  if (vkAllocateCommandBuffers(logicalDevice, &allocInfo, commandBuffers.data()) != VK_SUCCESS) {
    throw Logger::Errorf("vkAllocateCommandBuffers failed.");
  }
}

void Vulkan::RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex) {
  VkCommandBufferBeginInfo beginInfo{};
  beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  beginInfo.flags = 0;
  beginInfo.pInheritanceInfo = nullptr;

  if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
    throw Logger::Errorf("vkBeginCommandBuffer failed.");
  }

  VkRenderPassBeginInfo renderPassInfo{};
  renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
  renderPassInfo.renderPass = renderPass;
  renderPassInfo.framebuffer = swapChainFramebuffers[imageIndex];
  renderPassInfo.renderArea.offset = {0, 0};
  renderPassInfo.renderArea.extent = swapChainExtent;
  VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
  renderPassInfo.clearValueCount = 1;
  renderPassInfo.pClearValues = &clearColor;
  vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
  vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);

  std::vector<VkDeviceSize> offsets(vertexBuffers.size());
  for (u8 i = 0; i < vertexBuffers.size(); i++) {
    offsets[i] = 0;
  }
  vkCmdBindVertexBuffers(
      commandBuffer,
      0,
      vertexBuffers.size(),
      vertexBuffers.data(),
      offsets.data());
  vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT16);

  VkViewport viewport{};
  viewport.x = static_cast<f32>(viewportX);
  viewport.y = static_cast<f32>(viewportY);
  viewport.width = static_cast<f32>(viewportWidth);
  viewport.height = static_cast<f32>(viewportHeight);
  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;
  vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

  VkRect2D scissor{};
  scissor.offset = {0, 0};
  scissor.extent = swapChainExtent;
  vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

  vkCmdBindDescriptorSets(
      commandBuffer,
      VK_PIPELINE_BIND_POINT_GRAPHICS,
      pipelineLayout,
      0,
      1,
      &descriptorSets[currentFrame],
      0,
      nullptr);

  vkCmdDrawIndexed(commandBuffer, drawIndexCount, instanceCount, 0, 0, 0);

  vkCmdEndRenderPass(commandBuffer);

  if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
    throw Logger::Errorf("vkEndCommandBuffer failed.");
  }
}

void Vulkan::CreateSyncObjects() {
  imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
  renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
  inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

  VkSemaphoreCreateInfo semaphoreInfo{};
  semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

  for (uint8_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
    if (vkCreateSemaphore(logicalDevice, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) !=
        VK_SUCCESS) {
      throw Logger::Errorf("vkCreateSemaphore imageAvailableSemaphore failed.");
    }
    if (vkCreateSemaphore(logicalDevice, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) !=
        VK_SUCCESS) {
      throw Logger::Errorf("vkCreateSemaphore renderFinishedSemaphore failed.");
    }

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    // begin in signaled state; makes loop logic easy on first pass
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    if (vkCreateFence(logicalDevice, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS) {
      throw Logger::Errorf("vkCreateFence inFlightFence failed.");
    }
  }
}

void Vulkan::CreateBuffer(
    VkDeviceSize size,
    VkBufferUsageFlags usage,
    VkMemoryPropertyFlags properties,
    VkBuffer& buffer,
    VkDeviceMemory& bufferMemory) {
  VkBufferCreateInfo bufferInfo{};
  bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  bufferInfo.size = size;
  bufferInfo.usage = usage;
  bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

  if (vkCreateBuffer(logicalDevice, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
    throw Logger::Errorf("failed to create buffer!");
  }

  VkMemoryRequirements memRequirements;
  vkGetBufferMemoryRequirements(logicalDevice, buffer, &memRequirements);

  VkMemoryAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  allocInfo.allocationSize = memRequirements.size;
  allocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, properties);

  // TODO: The maximum number of simultaneous memory allocations is limited by the
  // maxMemoryAllocationCount So when objects on screen become too numerous, create a custom
  // allocator
  if (vkAllocateMemory(logicalDevice, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
    throw Logger::Errorf("failed to allocate buffer memory!");
  }

  vkBindBufferMemory(logicalDevice, buffer, bufferMemory, 0);
}

VkCommandBuffer Vulkan::BeginSingleTimeCommands() {
  VkCommandBufferAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  allocInfo.commandPool = commandPool;
  allocInfo.commandBufferCount = 1;

  VkCommandBuffer commandBuffer;
  vkAllocateCommandBuffers(logicalDevice, &allocInfo, &commandBuffer);

  VkCommandBufferBeginInfo beginInfo{};
  beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

  vkBeginCommandBuffer(commandBuffer, &beginInfo);

  return commandBuffer;
}

void Vulkan::EndSingleTimeCommands(VkCommandBuffer commandBuffer) {
  vkEndCommandBuffer(commandBuffer);

  VkSubmitInfo submitInfo{};
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &commandBuffer;

  vkQueueSubmit(pdqs.graphics.queue, 1, &submitInfo, VK_NULL_HANDLE);
  vkQueueWaitIdle(pdqs.graphics.queue);

  vkFreeCommandBuffers(logicalDevice, commandPool, 1, &commandBuffer);
}

void Vulkan::CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
  VkCommandBuffer commandBuffer = BeginSingleTimeCommands();

  VkBufferCopy copyRegion{};
  copyRegion.size = size;
  vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

  EndSingleTimeCommands(commandBuffer);
}

void Vulkan::TransitionImageLayout(
    VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout) {
  VkCommandBuffer commandBuffer = BeginSingleTimeCommands();

  VkImageMemoryBarrier barrier{};
  barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
  barrier.oldLayout = oldLayout;
  barrier.newLayout = newLayout;
  barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.image = image;
  barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  barrier.subresourceRange.baseMipLevel = 0;
  barrier.subresourceRange.levelCount = 1;
  barrier.subresourceRange.baseArrayLayer = 0;
  barrier.subresourceRange.layerCount = 1;

  VkPipelineStageFlags sourceStage;
  VkPipelineStageFlags destinationStage;

  if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

    sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
  } else if (
      oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
      newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
  } else {
    throw Logger::Errorf("unsupported layout transition!");
  }

  vkCmdPipelineBarrier(
      commandBuffer,
      sourceStage,
      destinationStage,
      0,
      0,
      nullptr,
      0,
      nullptr,
      1,
      &barrier);

  EndSingleTimeCommands(commandBuffer);
}

void Vulkan::CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) {
  VkCommandBuffer commandBuffer = BeginSingleTimeCommands();

  VkBufferImageCopy region{};
  region.bufferOffset = 0;
  region.bufferRowLength = 0;
  region.bufferImageHeight = 0;
  region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  region.imageSubresource.mipLevel = 0;
  region.imageSubresource.baseArrayLayer = 0;
  region.imageSubresource.layerCount = 1;
  region.imageOffset = {0, 0, 0};
  region.imageExtent = {width, height, 1};

  vkCmdCopyBufferToImage(
      commandBuffer,
      buffer,
      image,
      VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
      1,
      &region);

  EndSingleTimeCommands(commandBuffer);
}

void Vulkan::CreateVertexBuffer(u8 idx, u64 size, const void* indata) {
  VkDeviceSize bufferSize = size;

  VkBuffer stagingBuffer;
  VkDeviceMemory stagingBufferMemory;
  CreateBuffer(
      bufferSize,
      VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
      stagingBuffer,
      stagingBufferMemory);

  void* data;
  vkMapMemory(logicalDevice, stagingBufferMemory, 0, bufferSize, 0, &data);
  memcpy(data, indata, (size_t)bufferSize);
  vkUnmapMemory(logicalDevice, stagingBufferMemory);

  CreateBuffer(
      bufferSize,
      VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
      vertexBuffers[idx],
      vertexBufferMemories[idx]);

  CopyBuffer(stagingBuffer, vertexBuffers[idx], bufferSize);

  vkDestroyBuffer(logicalDevice, stagingBuffer, nullptr);
  vkFreeMemory(logicalDevice, stagingBufferMemory, nullptr);
}

void Vulkan::UpdateVertexBuffer(u8 idx, u64 size, const void* indata) {
  VkDeviceSize bufferSize = size;

  VkBuffer stagingBuffer;
  VkDeviceMemory stagingBufferMemory;
  CreateBuffer(
      bufferSize,
      VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
      stagingBuffer,
      stagingBufferMemory);

  void* data;
  vkMapMemory(logicalDevice, stagingBufferMemory, 0, bufferSize, 0, &data);
  memcpy(data, indata, (size_t)bufferSize);
  vkUnmapMemory(logicalDevice, stagingBufferMemory);

  CopyBuffer(stagingBuffer, vertexBuffers[idx], bufferSize);

  vkDestroyBuffer(logicalDevice, stagingBuffer, nullptr);
  vkFreeMemory(logicalDevice, stagingBufferMemory, nullptr);
}

uint32_t Vulkan::FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
  VkPhysicalDeviceMemoryProperties memProperties;
  vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

  for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
    if ((typeFilter & (1 << i)) &&
        (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
      return i;
    }
  }

  throw Logger::Errorf("failed to find suitable memory type!");
}

void Vulkan::CreateIndexBuffer(u64 size, const void* indata) {
  VkDeviceSize bufferSize = size;

  VkBuffer stagingBuffer;
  VkDeviceMemory stagingBufferMemory;
  CreateBuffer(
      bufferSize,
      VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
      stagingBuffer,
      stagingBufferMemory);

  void* data;
  vkMapMemory(logicalDevice, stagingBufferMemory, 0, bufferSize, 0, &data);
  memcpy(data, indata, (size_t)bufferSize);
  vkUnmapMemory(logicalDevice, stagingBufferMemory);

  CreateBuffer(
      bufferSize,
      VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
      indexBuffer,
      indexBufferMemory);

  CopyBuffer(stagingBuffer, indexBuffer, bufferSize);

  vkDestroyBuffer(logicalDevice, stagingBuffer, nullptr);
  vkFreeMemory(logicalDevice, stagingBufferMemory, nullptr);
}

void Vulkan::CreateUniformBuffers(const unsigned int length) {
  VkDeviceSize bufferSize = length;
  uniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
  uniformBufferLengths.resize(MAX_FRAMES_IN_FLIGHT);
  uniformBuffersMemory.resize(MAX_FRAMES_IN_FLIGHT);
  uniformBuffersMapped.resize(MAX_FRAMES_IN_FLIGHT);

  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
    uniformBufferLengths[i] = length;
    CreateBuffer(
        bufferSize,
        VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        uniformBuffers[i],
        uniformBuffersMemory[i]);

    vkMapMemory(logicalDevice, uniformBuffersMemory[i], 0, bufferSize, 0, &uniformBuffersMapped[i]);
  }
}

void Vulkan::UpdateUniformBuffer(uint32_t frame, void* ubo) {
  memcpy(uniformBuffersMapped[frame], ubo, uniformBufferLengths[frame]);
}

void Vulkan::CreateDescriptorPool() {
  std::array<VkDescriptorPoolSize, 2> poolSizes{};
  poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  poolSizes[0].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
  poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  poolSizes[1].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

  VkDescriptorPoolCreateInfo poolInfo{};
  poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
  poolInfo.pPoolSizes = poolSizes.data();
  poolInfo.maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

  if (vkCreateDescriptorPool(logicalDevice, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS) {
    throw Logger::Errorf("failed to create descriptor pool!");
  }
}

void Vulkan::CreateDescriptorSets() {
  std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, descriptorSetLayout);
  VkDescriptorSetAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  allocInfo.descriptorPool = descriptorPool;
  allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
  allocInfo.pSetLayouts = layouts.data();

  descriptorSets.resize(MAX_FRAMES_IN_FLIGHT);
  if (vkAllocateDescriptorSets(logicalDevice, &allocInfo, descriptorSets.data()) != VK_SUCCESS) {
    throw Logger::Errorf("failed to allocate descriptor sets!");
  }

  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
    VkDescriptorBufferInfo bufferInfo{};
    bufferInfo.buffer = uniformBuffers[i];
    bufferInfo.offset = 0;
    bufferInfo.range = uniformBufferLengths[i];

    VkDescriptorImageInfo imageInfo{};
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageInfo.imageView = textureImageView;
    imageInfo.sampler = textureSampler;

    std::array<VkWriteDescriptorSet, 2> descriptorWrites{};

    descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[0].dstSet = descriptorSets[i];
    descriptorWrites[0].dstBinding = 0;
    descriptorWrites[0].dstArrayElement = 0;
    descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorWrites[0].descriptorCount = 1;
    descriptorWrites[0].pBufferInfo = &bufferInfo;

    descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[1].dstSet = descriptorSets[i];
    descriptorWrites[1].dstBinding = 1;
    descriptorWrites[1].dstArrayElement = 0;
    descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptorWrites[1].descriptorCount = 1;
    descriptorWrites[1].pImageInfo = &imageInfo;

    vkUpdateDescriptorSets(
        logicalDevice,
        static_cast<uint32_t>(descriptorWrites.size()),
        descriptorWrites.data(),
        0,
        nullptr);
  }
}

/**
 * Load an image from disk. Queue it to Vulkan -> Buffer -> Image.
 */
void Vulkan::CreateTextureImage(const char* file) {
  int texWidth, texHeight, texChannels;
  stbi_uc* pixels = stbi_load(file, &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
  VkDeviceSize imageSize = texWidth * texHeight * 4;

  if (!pixels) {
    throw Logger::Errorf("failed to load texture image!");
  }

  VkBuffer stagingBuffer;
  VkDeviceMemory stagingBufferMemory;
  CreateBuffer(
      imageSize,
      VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
      stagingBuffer,
      stagingBufferMemory);

  void* data;
  vkMapMemory(logicalDevice, stagingBufferMemory, 0, imageSize, 0, &data);
  memcpy(data, pixels, static_cast<size_t>(imageSize));
  vkUnmapMemory(logicalDevice, stagingBufferMemory);

  stbi_image_free(pixels);

  CreateImage(
      texWidth,
      texHeight,
      VK_FORMAT_R8G8B8A8_SRGB,
      VK_IMAGE_TILING_OPTIMAL,
      VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
      textureImage,
      textureImageMemory);

  TransitionImageLayout(
      textureImage,
      VK_FORMAT_R8G8B8A8_SRGB,
      VK_IMAGE_LAYOUT_UNDEFINED,
      VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
  CopyBufferToImage(
      stagingBuffer,
      textureImage,
      static_cast<uint32_t>(texWidth),
      static_cast<uint32_t>(texHeight));
  TransitionImageLayout(
      textureImage,
      VK_FORMAT_R8G8B8A8_SRGB,
      VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
      VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

  vkDestroyBuffer(logicalDevice, stagingBuffer, nullptr);
  vkFreeMemory(logicalDevice, stagingBufferMemory, nullptr);
}

void Vulkan::CreateTextureImageView() {
  textureImageView = CreateImageView(textureImage, VK_FORMAT_R8G8B8A8_SRGB);
}

void Vulkan::CreateTextureSampler() {
  VkPhysicalDeviceProperties properties{};
  vkGetPhysicalDeviceProperties(physicalDevice, &properties);

  VkSamplerCreateInfo samplerInfo{};
  samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
  // nearest is best for pixel art, but a pixel shader is even better
  samplerInfo.magFilter = VK_FILTER_NEAREST;  // VK_FILTER_LINEAR;
  samplerInfo.minFilter = VK_FILTER_NEAREST;  // VK_FILTER_LINEAR;
  samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  samplerInfo.anisotropyEnable = VK_TRUE;
  samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
  samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
  samplerInfo.unnormalizedCoordinates = VK_FALSE;
  samplerInfo.compareEnable = VK_FALSE;
  samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
  samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

  if (vkCreateSampler(logicalDevice, &samplerInfo, nullptr, &textureSampler) != VK_SUCCESS) {
    throw Logger::Errorf("failed to create texture sampler!");
  }
}

VkImageView Vulkan::CreateImageView(VkImage image, VkFormat format) {
  VkImageViewCreateInfo viewInfo{};
  viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  viewInfo.image = image;
  viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
  viewInfo.format = format;
  viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  viewInfo.subresourceRange.baseMipLevel = 0;
  viewInfo.subresourceRange.levelCount = 1;
  viewInfo.subresourceRange.baseArrayLayer = 0;
  viewInfo.subresourceRange.layerCount = 1;

  VkImageView imageView;
  if (vkCreateImageView(logicalDevice, &viewInfo, nullptr, &imageView) != VK_SUCCESS) {
    throw Logger::Errorf("failed to create image view!");
  }

  return imageView;
}

void Vulkan::CreateImage(
    uint32_t width,
    uint32_t height,
    VkFormat format,
    VkImageTiling tiling,
    VkImageUsageFlags usage,
    VkMemoryPropertyFlags properties,
    VkImage& image,
    VkDeviceMemory& imageMemory) {
  VkImageCreateInfo imageInfo{};
  imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
  imageInfo.imageType = VK_IMAGE_TYPE_2D;
  imageInfo.extent.width = width;
  imageInfo.extent.height = height;
  imageInfo.extent.depth = 1;
  imageInfo.mipLevels = 1;
  imageInfo.arrayLayers = 1;
  imageInfo.format = format;
  imageInfo.tiling = tiling;
  imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  imageInfo.usage = usage;
  imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
  imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

  if (vkCreateImage(logicalDevice, &imageInfo, nullptr, &image) != VK_SUCCESS) {
    throw Logger::Errorf("failed to create image!");
  }

  VkMemoryRequirements memRequirements;
  vkGetImageMemoryRequirements(logicalDevice, image, &memRequirements);

  VkMemoryAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  allocInfo.allocationSize = memRequirements.size;
  allocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, properties);

  if (vkAllocateMemory(logicalDevice, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS) {
    throw Logger::Errorf("failed to allocate image memory!");
  }

  vkBindImageMemory(logicalDevice, image, imageMemory, 0);
}

void Vulkan::AwaitNextFrame() {
  if (vkWaitForFences(logicalDevice, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX) !=
      VK_SUCCESS) {
    throw Logger::Errorf("vkWaitForFences failed.");
  }

  VkResult result = vkAcquireNextImageKHR(
      logicalDevice,
      swapChain,
      UINT64_MAX,
      imageAvailableSemaphores[currentFrame],
      VK_NULL_HANDLE,
      &imageIndex);

  // detect window surface changed (ie. resized, color depth)
  if (result == VK_ERROR_OUT_OF_DATE_KHR) {
    // If the swap chain turns out to be out of date when attempting to acquire an image,
    // then it is no longer possible to present to it.
    // Therefore we should immediately recreate the swap chain
    RecreateSwapChain();
    // and try again in the next drawFrame call.
    return;
  } else if (result != VK_SUCCESS) {
    throw Logger::Errorf("vkAcquireNextImageKHR failed.");
  }
}

void Vulkan::DrawFrame() {
  // NOTICE: Fence will deadlock if waiting on an empty work queue.
  // Therefore, we only reset the fence just prior to submitting work.
  vkResetFences(logicalDevice, 1, &inFlightFences[currentFrame]);

  if (vkResetCommandBuffer(commandBuffers[currentFrame], 0) != VK_SUCCESS) {
    throw Logger::Errorf("vkResetCommandBuffer failed.");
  }

  RecordCommandBuffer(commandBuffers[currentFrame], imageIndex);

  VkSubmitInfo submitInfo{};
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

  VkSemaphore waitSemaphores[] = {imageAvailableSemaphores[currentFrame]};
  VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
  submitInfo.waitSemaphoreCount = 1;
  submitInfo.pWaitSemaphores = waitSemaphores;
  submitInfo.pWaitDstStageMask = waitStages;
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &commandBuffers[currentFrame];

  VkSemaphore signalSemaphores[] = {renderFinishedSemaphores[currentFrame]};
  submitInfo.signalSemaphoreCount = 1;
  submitInfo.pSignalSemaphores = signalSemaphores;

  if (vkQueueSubmit(pdqs.graphics.queue, 1, &submitInfo, inFlightFences[currentFrame]) !=
      VK_SUCCESS) {
    throw Logger::Errorf("vkQueueSubmit failed.");
  }

  VkPresentInfoKHR presentInfo{};
  presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
  presentInfo.waitSemaphoreCount = 1;
  presentInfo.pWaitSemaphores = signalSemaphores;

  VkSwapchainKHR swapChains[] = {swapChain};
  presentInfo.swapchainCount = 1;
  presentInfo.pSwapchains = swapChains;
  presentInfo.pImageIndices = &imageIndex;
  presentInfo.pResults = nullptr;
  bool same = pdqs.graphics.index.value() == pdqs.present.index.value();
  VkQueue queue;
  const char* err1;
  if (same) {
    queue = pdqs.graphics.queue;
    err1 = "vkQueuePresentKHR same failed.";
  } else {
    queue = pdqs.present.queue;
    err1 = "vkQueuePresentKHR !same failed.";
  }
  VkResult result2 = vkQueuePresentKHR(queue, &presentInfo);
  if (result2 == VK_ERROR_OUT_OF_DATE_KHR || result2 == VK_SUBOPTIMAL_KHR || framebufferResized) {
    framebufferResized = false;
    RecreateSwapChain();
  } else if (result2 != VK_SUCCESS) {
    throw Logger::Errorf(err1);
  }

  currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void Vulkan::DeviceWaitIdle() {
  vkDeviceWaitIdle(logicalDevice);
}

void Vulkan::RecreateSwapChain() {
  // TODO: the disadvantage of this approach is that we need to stop all rendering before creating
  // the new swap chain. It is possible to create a new swap chain while drawing commands on an
  // image from the old swap chain are still in-flight. You need to pass the previous swap chain to
  // the oldSwapChain field in the VkSwapchainCreateInfoKHR struct and destroy the old swap chain as
  // soon as you've finished using it.

  DeviceWaitIdle();

  CleanupSwapChain();

  CreateSwapChain();
  CreateImageViews();
  CreateFrameBuffers();
}

void Vulkan::CleanupSwapChain() {
  if (instance && logicalDevice && swapChain) {
    for (auto framebuffer : swapChainFramebuffers) {
      vkDestroyFramebuffer(logicalDevice, framebuffer, nullptr);
    }

    for (auto imageView : swapChainImageViews) {
      vkDestroyImageView(logicalDevice, imageView, nullptr);
    }

    vkDestroySwapchainKHR(logicalDevice, swapChain, nullptr);
  }
}

void Vulkan::Cleanup() {
  Logger::Infof("shutting down Vulkan.");

  if (instance) {
    if (logicalDevice) {
      CleanupSwapChain();

      vkDestroySampler(logicalDevice, textureSampler, nullptr);
      vkDestroyImageView(logicalDevice, textureImageView, nullptr);

      vkDestroyImage(logicalDevice, textureImage, nullptr);
      vkFreeMemory(logicalDevice, textureImageMemory, nullptr);

      if (descriptorPool) {
        vkDestroyDescriptorPool(logicalDevice, descriptorPool, nullptr);
      }

      for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroyBuffer(logicalDevice, uniformBuffers[i], nullptr);
        vkFreeMemory(logicalDevice, uniformBuffersMemory[i], nullptr);
      }

      if (descriptorSetLayout) {
        vkDestroyDescriptorSetLayout(logicalDevice, descriptorSetLayout, nullptr);
      }

      if (indexBuffer) {
        vkDestroyBuffer(logicalDevice, indexBuffer, nullptr);
      }
      if (indexBufferMemory) {
        vkFreeMemory(logicalDevice, indexBufferMemory, nullptr);
      }

      for (u8 i = 0; i < vertexBuffers.size(); i++) {
        vkDestroyBuffer(logicalDevice, vertexBuffers[i], nullptr);
      }
      for (u8 i = 0; i < vertexBufferMemories.size(); i++) {
        vkFreeMemory(logicalDevice, vertexBufferMemories[i], nullptr);
      }

      if (graphicsPipeline) {
        vkDestroyPipeline(logicalDevice, graphicsPipeline, nullptr);
      }
      if (pipelineLayout) {
        vkDestroyPipelineLayout(logicalDevice, pipelineLayout, nullptr);
      }
      if (renderPass) {
        vkDestroyRenderPass(logicalDevice, renderPass, nullptr);
      }

      for (uint8_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        if (renderFinishedSemaphores[i]) {
          vkDestroySemaphore(logicalDevice, renderFinishedSemaphores[i], nullptr);
        }
        if (imageAvailableSemaphores[i]) {
          vkDestroySemaphore(logicalDevice, imageAvailableSemaphores[i], nullptr);
        }
        if (inFlightFences[i]) {
          vkDestroyFence(logicalDevice, inFlightFences[i], nullptr);
        }
      }
      if (commandPool) {
        vkDestroyCommandPool(logicalDevice, commandPool, nullptr);
      }

      vkDestroyDevice(logicalDevice, nullptr);

      // if (enableValidationLayers) {
      //     DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
      // }

      if (surface) {
        vkDestroySurfaceKHR(instance, surface, nullptr);
      }
    }
    // NOTICE: physicalDevice is destroyed implicitly with instance.
    vkDestroyInstance(instance, nullptr);
  }
}

}  // namespace mks