#pragma once

#include <vulkan/vulkan.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <optional>
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

struct Vertex {
  glm::vec2 pos;
  glm::vec3 color;
  glm::vec2 texCoord;

  static VkVertexInputBindingDescription getBindingDescription();
  static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions();
};

class Vulkan {
 public:
  static const int MAX_FRAMES_IN_FLIGHT = 2;

  /**
   * Query the instance for a list of validation layers it supports.
   */
  static const bool CheckInstanceLayers(const std::vector<const char*> requiredLayers);

  /**
   * Query the instance to verify it supports our list of required extensions.
   *
   * @param requiredExtensions - list of required extensions
   * @return whether all required extensions are supported
   */
  static const bool CheckInstanceExtensions(const std::vector<const char*> requiredExtensions);

  Vulkan();

  /**
   * The very first thing you need to do is initialize the Vulkan library by creating an instance.
   * The instance is the connection between your application and the Vulkan library and creating it
   * involves specifying some details about your application to the driver.
   *
   * @param name - Application name
   * @param major - Application major version number
   * @param minor - Application minor version number
   * @param hotfix  - Application hotfix version number
   */
  Vulkan(
      const char* name,
      const unsigned int major,
      const unsigned int minor,
      const unsigned int hotfix,
      const std::vector<const char*> requiredValidationLayers,
      const std::vector<const char*> requiredExtensionNames);

  ~Vulkan();

  // list of required validation layers, according to developer debug preferences
  std::vector<const char*> requiredValidationLayers;

  /**
   * Abort unless all required Vulkan validation layers
   * are supported by the Vulkan driver.
   */
  void AssertDriverValidationLayersSupported();

  /**
   * Abort unless all extensions requested by the window surface
   * are supported by the Vulkan driver.
   *
   * @param requiredExtensionNames - list of required extensions.
   */
  void AssertDriverExtensionsSupported(std::vector<const char*> requiredExtensionNames);

  /**
   * Select the physical device we will use for subsequent calls.
   *
   * @param deviceIndex - Which GPU to choose.
   */
  const void UsePhysicalDevice(const unsigned int deviceIndex);

  /**
   * Locate all instances of each queue type,
   * across all queue families on the current physical device.
   */
  void LocateQueueFamilies();

  /**
   * Verify required extensions are present on current physical device.
   *
   * @return - Whether support was found for all in list.
   */
  const bool CheckPhysicalDeviceExtensions(
      std::vector<const char*> requiredPhysicalDeviceExtensions) const;

  /**
   * Verify whether physical device supports Swap Chain feature.
   */
  const void AssertSwapChainSupport(std::vector<const char*> requiredPhysicalDeviceExtensions);

  /**
   * Set the current logical device, which will use the current physical device.
   */
  void UseLogicalDevice(std::vector<const char*> requiredPhysicalDeviceExtensions);

  /**
   * Construct the swap chain.
   */
  void CreateSwapChain();

  void CreateShaderModule(const std::vector<char>& code, VkShaderModule* mod) const;
  void DestroyShaderModule(const VkShaderModule* shaderModule) const;

  void CreateImageViews();
  void CreateRenderPass();
  void CreateGraphicsPipeline();
  void CreateDescriptorSetLayout();
  void CreateFrameBuffers();
  void CreateCommandPool();
  void CreateCommandBuffers();
  const void SetVertexBufferData(
      const std::vector<mks::Vertex> vertices, const std::vector<uint16_t> indices);
  void RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);
  void CreateSyncObjects();
  void CreateBuffer(
      VkDeviceSize size,
      VkBufferUsageFlags usage,
      VkMemoryPropertyFlags properties,
      VkBuffer& buffer,
      VkDeviceMemory& bufferMemory);
  VkCommandBuffer BeginSingleTimeCommands();
  void EndSingleTimeCommands(VkCommandBuffer commandBuffer);
  void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
  void TransitionImageLayout(
      VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
  void CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
  void CreateVertexBuffer();
  void UpdateVertexBuffer();
  void CreateIndexBuffer();
  void CreateUniformBuffers(const unsigned int length);
  void UpdateUniformBuffer(uint32_t frame, void* data);
  void CreateDescriptorPool();
  void CreateDescriptorSets();
  uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
  void CreateTextureImage(const char* file);
  void CreateTextureImageView();
  void CreateTextureSampler();
  VkImageView CreateImageView(VkImage image, VkFormat format);
  void CreateImage(
      uint32_t width,
      uint32_t height,
      VkFormat format,
      VkImageTiling tiling,
      VkImageUsageFlags usage,
      VkMemoryPropertyFlags properties,
      VkImage& image,
      VkDeviceMemory& imageMemory);
  void AwaitNextFrame();
  void DrawFrame();
  void DeviceWaitIdle();
  void RecreateSwapChain();
  void CleanupSwapChain();
  void Cleanup();

  VkInstance instance = nullptr;
  VkSurfaceKHR surface = 0;
  PhysicalDeviceQueues pdqs = PhysicalDeviceQueues{};
  uint32_t width, height = 0;
  bool framebufferResized = false;
  bool minimized = false;
  uint32_t imageIndex = 0;
  uint8_t currentFrame = 0;
  VkExtent2D swapChainExtent = {};

 private:
  VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
  VkDevice logicalDevice = nullptr;
  // TODO: swap chain stuff should get its own struct
  SwapChainSupportDetails swapChainSupport = {};
  VkSwapchainKHR swapChain = 0;
  std::vector<VkImage> swapChainImages = {};
  VkFormat swapChainImageFormat = {};
  std::vector<VkImageView> swapChainImageViews = {};
  VkRenderPass renderPass = {};
  VkDescriptorSetLayout descriptorSetLayout;
  VkPipelineLayout pipelineLayout = {};
  VkPipeline graphicsPipeline = {};
  std::vector<VkFramebuffer> swapChainFramebuffers = {};
  VkCommandPool commandPool = {};
  std::vector<VkCommandBuffer> commandBuffers = {};
  std::vector<VkSemaphore> imageAvailableSemaphores;
  std::vector<VkSemaphore> renderFinishedSemaphores;
  std::vector<VkFence> inFlightFences;
  VkBuffer vertexBuffer;
  VkDeviceMemory vertexBufferMemory;
  VkBuffer indexBuffer;
  VkDeviceMemory indexBufferMemory;
  std::vector<VkBuffer> uniformBuffers;
  std::vector<unsigned int> uniformBufferLengths;
  std::vector<VkDeviceMemory> uniformBuffersMemory;
  std::vector<void*> uniformBuffersMapped;
  VkDescriptorPool descriptorPool;
  std::vector<VkDescriptorSet> descriptorSets;
  VkImage textureImage;
  VkDeviceMemory textureImageMemory;
  VkImageView textureImageView;
  VkSampler textureSampler;
  std::vector<mks::Vertex> vertices;
  std::vector<uint16_t> indices;
};

}  // namespace mks