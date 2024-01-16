
#include <chrono>
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

}  // namespace

int main() {
  try {
    mks::Logger::Infof("Begin SDL2 Vulkan test.");

    auto w = mks::Window{};
    w.Begin("Vulkan_test", "SDL2 Vulkan Window", 800, 600);

    w.v.CreateImageViews();
    w.v.CreateRenderPass();
    w.v.CreateDescriptorSetLayout();
    w.v.CreateGraphicsPipeline();
    w.v.CreateFrameBuffers();
    w.v.CreateCommandPool();
    w.v.CreateTextureImage();
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

    w.RenderLoop(60, [&w, &ubo]() {
      static auto startTime = std::chrono::high_resolution_clock::now();

      auto currentTime = std::chrono::high_resolution_clock::now();
      float time =
          std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime)
              .count();
      ubo.model =
          glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
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