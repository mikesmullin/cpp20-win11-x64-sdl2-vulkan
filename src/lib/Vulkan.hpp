#pragma once

#include <vulkan/vulkan.h>

#include <memory>
#include <optional>
#include <set>
#include <vector>

/**
 * Enables Vulkan validation layer handler
 * (performance penalty) and corresponding stderr log output.
 *
 * NOTICE: Validation layers can only be used if they have been installed onto the system. For
 * example, the LunarG validation layers are only available on PCs with the Vulkan SDK installed.
 */
#define DEBUG_VULKAN

namespace mks {

/**
 * One possible type of queue, and
 * the queue family indices where it was found
 * on the current physical device.
 */
struct PhysicalDeviceQueue {
  bool required;
  const bool supported() const {
    return familyIndices.size() > 0;
  };
  const bool valid() const {
    return !required || supported();
  };
  unsigned int selected = 0;
  const uint32_t selectedFamilyIndex() const {
    return supported() ? familyIndices[selected] : 0;
  }
  const bool selectedFamilyIndexEqual(const uint32_t idx) const {
    return supported() && idx == familyIndices[selected];
  };
  std::vector<uint32_t> familyIndices;
  VkQueue queue;
};

struct PhysicalDeviceQueues {
  PhysicalDeviceQueue graphics;
  PhysicalDeviceQueue compute;
  PhysicalDeviceQueue transfer;
  PhysicalDeviceQueue sparse;
  PhysicalDeviceQueue protect;
  PhysicalDeviceQueue optical;
  PhysicalDeviceQueue present;
};

class Vulkan {
 public:
  /**
   * Query the driver for a list of validation layers it supports.
   *
   * @return bool
   */
  static const bool CheckLayers(const std::vector<const char*> requiredLayers);

  /**
   * Query the driver for a list of extensions it supports.
   *
   * @return const bool
   */
  static const bool CheckExtensions(const std::vector<const char*> requiredExtensions);

  /**
   * The very first thing you need to do is initialize the Vulkan library by creating an instance.
   * The instance is the connection between your application and the Vulkan library and creating it
   * involves specifying some details about your application to the driver.
   *
   * @param name - Application name
   * @param major - Application major version number
   * @param minor - Application minor version number
   * @param hotfix  - Application hotfix version number
   * @param requiredValidationLayers - list of validation layer names to validate and enable.
   * @param requiredExtensionNames - list of physical device extension names to validate and enable.
   */
  Vulkan(
      const char* name,
      const unsigned int major,
      const unsigned int minor,
      const unsigned int hotfix,
      const std::vector<const char*> requiredValidationLayers,
      const std::vector<const char*> requiredExtensionNames);

  ~Vulkan();

  /**
   * Select the physical device we will use for subsequent calls.
   *
   * @param deviceIndex - Which GPU to choose.
   * @return - Whether device exists, and was found to be compatible.
   */
  const bool UsePhysicalDevice(const unsigned int deviceIndex);

  /**
   * Validate desired queue families are supported by the current physical device.
   */
  const bool CheckQueues();
  void BeginGetLogicalDeviceQueues(const std::set<PhysicalDeviceQueue> queues) const;
  const bool EndGetLogicalDeviceQueue(const PhysicalDeviceQueue queue);

  void UseLogicalDevice(const std::vector<const char*> requiredValidationLayers);
  void CreateWindowSurface();
  const bool CheckPhysicalDeviceSurfaceSupports() const;

  VkInstance instance = nullptr;
  VkSurfaceKHR surface = nullptr;
  PhysicalDeviceQueues pdqfs = PhysicalDeviceQueues{};

 private:
  VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
  VkDevice logicalDevice = nullptr;
};

}  // namespace mks