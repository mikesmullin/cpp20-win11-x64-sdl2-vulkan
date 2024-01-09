#include "Vulkan.hpp"

#include <cstdint>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <algorithm>
#include <array>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <memory>
#include <vector>

#include "Logger.hpp"
#include "Shader.hpp"

namespace {

const int MAX_FRAMES_IN_FLIGHT = 2;

struct Vertex {
  glm::vec2 pos;
  glm::vec3 color;

  static VkVertexInputBindingDescription getBindingDescription() {
    VkVertexInputBindingDescription bindingDescription{};
    bindingDescription.binding = 0;
    bindingDescription.stride = sizeof(Vertex);
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    return bindingDescription;
  }

  static std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions() {
    std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions{};

    attributeDescriptions[0].binding = 0;
    attributeDescriptions[0].location = 0;
    attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
    attributeDescriptions[0].offset = offsetof(Vertex, pos);

    attributeDescriptions[1].binding = 0;
    attributeDescriptions[1].location = 1;
    attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[1].offset = offsetof(Vertex, color);

    return attributeDescriptions;
  }
};

const std::vector<Vertex> vertices = {
    {{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
    {{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
    {{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
    {{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}}};

const std::vector<uint16_t> indices = {0, 1, 2, 2, 3, 0};

}  // namespace

namespace mks {

void Vulkan::AssertDriverValidationLayersSupported() {
#ifdef DEBUG_VULKAN
  requiredValidationLayers.resize(1);
  // SDK-provided layer conveniently bundles all useful standard validation
  requiredValidationLayers[0] = "VK_LAYER_KHRONOS_validation";

  bool supported = Vulkan::CheckInstanceLayers(Vulkan::requiredValidationLayers);
  if (!supported) {
    throw Logger::Errorf("Missing required Vulkan validation layers.");
  }

  // TODO: Device-specific layer validation is deprecated. However,
  //   the specification document still recommends that you enable validation layers at device
  //   level as well for compatibility, and it's required by some implementations. we'll see this
  //   code later on:
  //   https://vulkan-tutorial.com/en/Drawing_a_triangle/Setup/Logical_device_and_queues
#endif
}

void Vulkan::AssertDriverExtensionsSupported(std::vector<const char*> requiredExtensionNames) {
  bool supported = false;
  supported = Vulkan::CheckInstanceExtensions(requiredExtensionNames);
  if (!supported) {
    throw Logger::Errorf("Vulkan driver is missing required Vulkan extensions.");
  }
}

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
    throw Logger::Errorf("Failed to create Vulkan instance.");
  }
}

Vulkan::~Vulkan() {
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
    if (i == requiredDeviceIndex && discrete && geometry) {
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

const bool Vulkan::CheckPhysicalDeviceExtensions(
    std::vector<const char*> requiredPhysicalDeviceExtensions) const {
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

const void Vulkan::AssertSwapChainSupport(
    std::vector<const char*> requiredPhysicalDeviceExtensions) {
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

  bool supported = extensionsSupported && !swapChainSupport.formats.empty() &&
                   !swapChainSupport.presentModes.empty();
  if (!supported) {
    throw mks::Logger::Errorf("Missing swap chain support on physical device.");
  }
}

void Vulkan::UseLogicalDevice(std::vector<const char*> requiredPhysicalDeviceExtensions) {
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

  // TODO: Need support for macOS retina displays which means the surface resolution will be
  // larger than the window resolution.

  // TODO: these bounds need to be restricted within the maximum capability of the surface and
  // device, whichever is smaller. Unfortunately I was not able to make that work using
  // swapChainSupport.capabilities because (min|max)ImageExtent always returns the current bounds,
  // which is not helpful. In the event that Vulkan notifies us of a surface change, which is not
  // preceded by an SDL resize event. (e.g.,. color-depth change?) Then we are missing a function to
  // determine the new bounds. So we risk a crash here!
  VkExtent2D extent;
  extent.width = width;
  extent.height = height;

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

  VkRenderPassCreateInfo renderPassInfo{};
  renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
  renderPassInfo.attachmentCount = 1;
  renderPassInfo.pAttachments = &colorAttachment;
  renderPassInfo.subpassCount = 1;
  renderPassInfo.pSubpasses = &subpass;

  VkSubpassDependency dependency{};
  dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
  dependency.dstSubpass = 0;
  dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  dependency.srcAccessMask = 0;
  dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

  renderPassInfo.dependencyCount = 1;
  renderPassInfo.pDependencies = &dependency;

  if (vkCreateRenderPass(logicalDevice, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
    throw Logger::Errorf("vkCreateRenderPass failed.");
  }
}

void Vulkan::CreateGraphicsPipeline() {
  // NOTICE: pipeline state is immutable; you will make many of these instances

  Shader s = {};
  // TODO: Make shaders configurable via input
  auto shader1 = s.readFile("../assets/shaders/simple_shader.frag.spv");
  auto shader2 = s.readFile("../assets/shaders/simple_shader.vert.spv");

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

  auto bindingDescription = Vertex::getBindingDescription();
  auto attributeDescriptions = Vertex::getAttributeDescriptions();
  VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
  vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
  vertexInputInfo.vertexBindingDescriptionCount = 1;
  vertexInputInfo.vertexAttributeDescriptionCount =
      static_cast<uint32_t>(attributeDescriptions.size());
  vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;             // Optional
  vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();  // Optional

  // specify what kind of geometry will be drawn from the vertices, and
  // if primitive restart should be enabled.
  VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
  inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
  inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
  inputAssembly.primitiveRestartEnable = VK_FALSE;

  // A viewport basically describes the region of the framebuffer that the output will be rendered
  // to.
  VkViewport viewport{};
  viewport.x = 0.0f;
  viewport.y = 0.0f;
  viewport.width = (float)swapChainExtent.width;
  viewport.height = (float)swapChainExtent.height;
  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;

  // scissor rectangles define in which regions pixels will actually be stored.
  // pixels outside the scissor rectangles will be discarded by the rasterizer.
  VkRect2D scissor{};
  scissor.offset = {0, 0};
  scissor.extent = swapChainExtent;

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
  rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
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
  colorBlendAttachment.blendEnable = VK_FALSE;
  colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
  colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
  colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
  colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
  colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
  colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

  VkPipelineColorBlendStateCreateInfo colorBlending{};
  colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
  colorBlending.logicOpEnable = VK_FALSE;
  colorBlending.logicOp = VK_LOGIC_OP_COPY;
  colorBlending.attachmentCount = 1;
  colorBlending.pAttachments = &colorBlendAttachment;
  colorBlending.blendConstants[0] = 0.0f;
  colorBlending.blendConstants[1] = 0.0f;
  colorBlending.blendConstants[2] = 0.0f;
  colorBlending.blendConstants[3] = 0.0f;

  VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
  pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  pipelineLayoutInfo.setLayoutCount = 0;
  pipelineLayoutInfo.pSetLayouts = nullptr;
  pipelineLayoutInfo.pushConstantRangeCount = 0;
  pipelineLayoutInfo.pPushConstantRanges = nullptr;

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
  allocInfo.commandPool = commandPool;
  allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
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

  VkBuffer vertexBuffers[] = {vertexBuffer};
  VkDeviceSize offsets[] = {0};
  vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
  vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT16);

  VkViewport viewport{};
  viewport.x = 0.0f;
  viewport.y = 0.0f;
  viewport.width = static_cast<float>(swapChainExtent.width);
  viewport.height = static_cast<float>(swapChainExtent.height);
  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;
  vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

  VkRect2D scissor{};
  scissor.offset = {0, 0};
  scissor.extent = swapChainExtent;
  vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

  vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);

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

void Vulkan::CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
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

  VkBufferCopy copyRegion{};
  copyRegion.size = size;
  vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

  vkEndCommandBuffer(commandBuffer);

  VkSubmitInfo submitInfo{};
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &commandBuffer;

  vkQueueSubmit(pdqs.graphics.queue, 1, &submitInfo, VK_NULL_HANDLE);
  vkQueueWaitIdle(pdqs.graphics.queue);

  vkFreeCommandBuffers(logicalDevice, commandPool, 1, &commandBuffer);
}

void Vulkan::CreateVertexBuffer() {
  VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

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
  memcpy(data, vertices.data(), (size_t)bufferSize);
  vkUnmapMemory(logicalDevice, stagingBufferMemory);

  CreateBuffer(
      bufferSize,
      VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
      vertexBuffer,
      vertexBufferMemory);

  CopyBuffer(stagingBuffer, vertexBuffer, bufferSize);

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

void Vulkan::CreateIndexBuffer() {
  VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();

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
  memcpy(data, indices.data(), (size_t)bufferSize);
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

void Vulkan::DrawFrame() {
  if (vkWaitForFences(logicalDevice, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX) !=
      VK_SUCCESS) {
    throw Logger::Errorf("vkWaitForFences failed.");
  }

  uint32_t imageIndex;
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

      if (indexBuffer) {
        vkDestroyBuffer(logicalDevice, indexBuffer, nullptr);
      }
      if (indexBufferMemory) {
        vkFreeMemory(logicalDevice, indexBufferMemory, nullptr);
      }

      if (vertexBuffer) {
        vkDestroyBuffer(logicalDevice, vertexBuffer, nullptr);
      }
      if (vertexBufferMemory) {
        vkFreeMemory(logicalDevice, vertexBufferMemory, nullptr);
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