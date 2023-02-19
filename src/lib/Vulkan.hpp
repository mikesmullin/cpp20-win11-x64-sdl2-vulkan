#pragma once

#include <vulkan/vulkan.h>

#include <memory>
#include <optional>
#include <set>
#include <string>
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
  /**
   * Selected queue family index.
   */
  std::optional<uint32_t> index;
  VkQueue queue = nullptr;
};

struct PhysicalDeviceQueues {
  PhysicalDeviceQueue graphics;
  PhysicalDeviceQueue present;
};

struct SwapChainSupportDetails {
  VkSurfaceCapabilitiesKHR capabilities;
  std::vector<VkSurfaceFormatKHR> formats;
  std::vector<VkPresentModeKHR> presentModes;
};

class Vulkan {
 public:
  /**
   * Query the instance for a list of validation layers it supports.
   *
   * @return bool
   */
  static const bool CheckInstanceLayers(const std::vector<const char*> requiredLayers);

  /**
   * Query the instance for a list of extensions it supports.
   *
   * @return const bool
   */
  static const bool CheckInstanceExtensions(const std::vector<const char*> requiredExtensions);

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
   * Locate all instances of each queue type,
   * across all queue families on the current physical device.
   */
  void LocateQueueFamilies();

  /**
   * Verify required extensions are present on current physical device.
   *
   * @param requiredExtensions - List of extension names.
   * @return - Whether support was found for all in list.
   */
  const bool CheckPhysicalDeviceExtensions(const std::vector<const char*> requiredExtensions) const;

  /**
   * Verify whether physical device supports Swap Chain feature.
   *
   * @return - Whether support was found.
   */
  const bool CheckSwapChainSupport();

  /**
   * Set the current logical device, which will use the current physical device.
   *
   * @param requiredValidationLayers - Enable these validation layers on the device (for
   * backward-compatibility).
   * @param requiredPhysicalDeviceExtensions - Enable these device extensions.
   */
  void UseLogicalDevice(
      const std::vector<const char*> requiredValidationLayers,
      std::vector<const char*> requiredPhysicalDeviceExtensions);

  /**
   * Construct the swap chain.
   */
  void CreateSwapChain();

  void CreateShaderModule(const std::vector<char>& code, VkShaderModule* mod) const;
  void DestroyShaderModule(const VkShaderModule* shaderModule) const;

  void CreateImageViews();
  void CreateRenderPass();
  void CreateGraphicsPipeline();
  void CreateFrameBuffers();
  void CreateCommandPool();
  void CreateCommandBuffer();
  void RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);
  void CreateSyncObjects();
  void DrawFrame();
  void DeviceWaitIdle();

  VkInstance instance = nullptr;
  VkSurfaceKHR surface = nullptr;
  PhysicalDeviceQueues pdqs = PhysicalDeviceQueues{};
  uint32_t width, height = 0;

 private:
  VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
  VkDevice logicalDevice = nullptr;
  // TODO: swap chain stuff should get its own struct
  SwapChainSupportDetails swapChainSupport = {};
  VkSwapchainKHR swapChain = nullptr;
  std::vector<VkImage> swapChainImages = {};
  VkFormat swapChainImageFormat = {};
  VkExtent2D swapChainExtent = {};
  std::vector<VkImageView> swapChainImageViews = {};
  VkRenderPass renderPass = {};
  VkPipelineLayout pipelineLayout = {};
  VkPipeline graphicsPipeline = {};
  std::vector<VkFramebuffer> swapChainFramebuffers = {};
  VkCommandPool commandPool = {};
  VkCommandBuffer commandBuffer = {};
  VkSemaphore imageAvailableSemaphore;
  VkSemaphore renderFinishedSemaphore;
  VkFence inFlightFence;
};

}  // namespace mks