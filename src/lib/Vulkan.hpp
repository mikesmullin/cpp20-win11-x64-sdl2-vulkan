#pragma once

#include <vulkan/vulkan.h>

#include <memory>
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

struct VulkanExtensionSupport {
  bool allRequiredSupported;
  std::vector<const char*> required;
  std::vector<VkExtensionProperties> supported;
  std::vector<const char*> missing;
};

class Vulkan {
 public:
  Vulkan();
  ~Vulkan();

  /**
   * The very first thing you need to do is initialize the Vulkan library by creating an instance.
   * The instance is the connection between your application and the Vulkan library and creating it
   * involves specifying some details about your application to the driver.
   *
   * @param appInfo - Application info struct returned by CreateInstance().
   */
  void Vulkan::CreateInstance(
      std::unique_ptr<VkApplicationInfo> appInfo, std::vector<const char*> extensionNames);

  /**
   * Define struct specifying some details about your application.
   * To be passed to the driver via CreateInstance().
   *
   * @param name - Application name
   * @param major - Application major version number
   * @param minor - Application minor version number
   * @param hotfix  - Application hotfix version number
   * @return std::unique_ptr<VkApplicationInfo>
   */
  std::unique_ptr<VkApplicationInfo> Vulkan::DescribeApplication(
      const char* name,
      const unsigned int major,
      const unsigned int minor,
      const unsigned int hotfix);

  /**
   * Query the device for a list of extensions it supports.
   *
   * @return std::vector<VkExtensionProperties>
   */
  VulkanExtensionSupport Vulkan::CheckSupportedExtensions(
      std::vector<const char*> requiredExtensions);

 private:
  VkInstance instance;
};
}  // namespace mks