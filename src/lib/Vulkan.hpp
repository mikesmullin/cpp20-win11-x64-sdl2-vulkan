#pragma once

#include <vulkan/vulkan.h>

#include <memory>
#include <optional>
#include <vector>

/**
 * Enables Vulkan validation layer handler
 * (performance penalty) and correspoding stderr log output.
 *
 * NOTICE: Validation layers can only be used if they have been installed onto the system. For
 * example, the LunarG validation layers are only available on PCs with the Vulkan SDK installed.
 */
#define DEBUG_VULKAN

namespace mks {

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
  void CheckQueues();

  void UseLogicalDevice(const std::vector<const char*> requiredValidationLayers);
  void CreateWindowSurface();
  const bool CheckPhysicalDeviceSurfaceSupports() const;

  VkInstance instance = nullptr;
  VkSurfaceKHR surface = nullptr;

 private:
  VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
  VkDevice logicalDevice = nullptr;
  std::optional<uint32_t> graphicsQueueFamilyIndex = std::nullopt;
  std::optional<uint32_t> presentQueueFamilyIndex = std::nullopt;
  VkQueue graphicsQueue = nullptr;
  VkQueue presentQueue = nullptr;
};
}  // namespace mks