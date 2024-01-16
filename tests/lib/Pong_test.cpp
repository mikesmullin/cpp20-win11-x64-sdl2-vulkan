
#include <iostream>
#include <stdexcept>

#include "../../src/lib/Logger.hpp"
#include "../../src/lib/Window.hpp"

int main() {
  try {
    mks::Logger::Infof("Begin Pong test.");

    auto w = mks::Window{};
    w.Begin("Pong_test", "Pong", 1024, 768);

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
    w.v.CreateUniformBuffers();
    w.v.CreateDescriptorPool();
    w.v.CreateDescriptorSets();
    w.v.CreateCommandBuffers();
    w.v.CreateSyncObjects();

    w.RenderLoop(60, []() {

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