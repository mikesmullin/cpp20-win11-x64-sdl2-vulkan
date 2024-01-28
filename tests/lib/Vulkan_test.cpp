
#include <iostream>
#include <stdexcept>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "../../src/lib/Logger.hpp"
#include "../../src/lib/Window.hpp"

namespace {

struct ubo_MVPMatrix {
  glm::mat4 model;
  glm::mat4 view;
  glm::mat4 proj;
};

const std::vector<mks::Vertex> vertices = {
    {{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {0.2f, 0.0f}},
    {{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
    {{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.25f}},
    {{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}, {0.2f, 0.25f}}};

const std::vector<uint16_t> indices = {0, 1, 2, 2, 3, 0};

}  // namespace

int main() {
  try {
    mks::Logger::Infof("Begin SDL2 Vulkan test.");

    auto w = mks::Window{};
    w.Begin("SDL2 Vulkan Window", 800, 600);
    w.v.AssertDriverValidationLayersSupported();
    w.v.AssertDriverExtensionsSupported(w.requiredExtensionNames);
    w.v.CreateInstance("Vulkan_test", 1, 0, 0);
    w.v.UsePhysicalDevice(0);
    w.Bind();
    auto b = w.GetDrawableAreaExtentBounds();
    w.v.width = b.width;
    w.v.height = b.height;

    w.v.InitSwapChain();
    w.v.SetVertexBufferData(vertices, indices);

    w.v.CreateImageViews();
    w.v.CreateRenderPass();
    w.v.CreateDescriptorSetLayout();
    w.v.CreateGraphicsPipeline();
    w.v.CreateFrameBuffers();
    w.v.CreateCommandPool();
    w.v.CreateTextureImage("../assets/textures/crates.png");
    w.v.CreateTextureImageView();
    w.v.CreateTextureSampler();
    w.v.CreateVertexBuffer();
    w.v.CreateIndexBuffer();
    w.v.CreateUniformBuffers(sizeof(ubo_MVPMatrix));
    w.v.CreateDescriptorPool();
    w.v.CreateDescriptorSets();
    w.v.CreateCommandBuffers();
    w.v.CreateSyncObjects();

    ubo_MVPMatrix ubo{};  // model_view_projection_matrix

    w.RenderLoop(
        60,
        [](auto& e) {},
        [&w, &ubo](float deltaTime) {
          static float angle = 0.0f;
          static const float ROT_SPEED = 0.05f;  // deg per sec
          angle += (ROT_SPEED * deltaTime);

          ubo.model = glm::rotate(
              glm::mat4(1.0f),
              angle * glm::radians(90.0f),
              glm::vec3(0.0f, 0.0f, 1.0f));
          ubo.view = glm::lookAt(
              glm::vec3(2.0f, 2.0f, 2.0f),
              glm::vec3(0.0f, 0.0f, 0.0f),
              glm::vec3(0.0f, 0.0f, 1.0f));
          ubo.proj = glm::perspective(
              glm::radians(45.0f),
              w.v.swapChainExtent.width / (float)w.v.swapChainExtent.height,
              0.1f,
              10.0f);
          ubo.proj[1][1] *= -1;

          w.v.UpdateUniformBuffer(w.v.currentFrame, &ubo);
        });

    w.v.DeviceWaitIdle();
    w.v.Cleanup();
    w.End();

    mks::Logger::Infof("End of test.");
  } catch (const std::runtime_error& e) {
    std::cerr << "Fatal: " << e.what() << '\n';
    return EXIT_FAILURE;
  } catch (...) {
    std::cerr << "Unexpected error\n";
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}