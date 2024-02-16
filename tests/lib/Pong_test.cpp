#include <functional>
#include <iostream>
#include <stdexcept>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "../../src/lib/Audio.hpp"
#include "../../src/lib/Base.hpp"
#include "../../src/lib/Gamepad.hpp"
#include "../../src/lib/Logger.hpp"
#include "../../src/lib/Lua.hpp"
#include "../../src/lib/Window.hpp"

namespace {

struct ubo_MVPMatrix {
  glm::mat4 model;
  glm::mat4 view;
  glm::mat4 proj;
};

std::vector<mks::Vertex> vertices = {
    {{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {0.2f, 0.0f}},
    {{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
    {{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.25f}},
    {{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}, {0.2f, 0.25f}}};

std::vector<uint16_t> indices = {0, 1, 2, 2, 3, 0};

mks::Audio a{};

int lua_LoadSoundEffect(lua_State* L) {
  auto file = lua_tostring(L, 1);
  a.addSoundEffect(file);
  return 0;
}

int lua_PlaySoundEffect(lua_State* L) {
  const int index = lua_tointeger(L, 1);
  a.playSoundEffect(index);
  return 0;
}

int lua_GetGamepadInput(lua_State* L) {
  const int index = lua_tointeger(L, 1);
  auto g = mks::Gamepad::registry[index];
  lua_pushnumber(L, g->axes[0]);
  lua_pushnumber(L, g->axes[1]);
  lua_pushnumber(L, g->axes[2]);
  lua_pushnumber(L, g->axes[3]);
  lua_pushboolean(L, g->buttons[0]);
  lua_pushboolean(L, g->buttons[1]);
  lua_pushboolean(L, g->buttons[2]);
  lua_pushboolean(L, g->buttons[3]);
  return 8;
}

std::vector<std::string> textureFiles;
int lua_LoadTexture(lua_State* L) {
  textureFiles.resize(1);
  auto file = lua_tostring(L, 1);
  // TODO: make it possible to load more than one texture atlas?
  textureFiles[0] = static_cast<std::string>(file);
  return 0;
}

mks::Window* ww;
bool isVBODirty;
int lua_AdjustVBO(lua_State* L) {
  auto u = lua_tointeger(L, 1);
  auto v = lua_tointeger(L, 2);
  vertices[0].texCoord[0] += 0.2f;
  vertices[0].texCoord[1] += 0.25f;
  vertices[1].texCoord[0] += 0.2f;
  vertices[1].texCoord[1] += 0.25f;
  vertices[2].texCoord[0] += 0.2f;
  vertices[2].texCoord[1] += 0.25f;
  vertices[3].texCoord[0] += 0.2f;
  vertices[3].texCoord[1] += 0.25f;
  ww->v.SetVertexBufferData(vertices, indices);
  isVBODirty = true;
  return 0;
}

}  // namespace

int main() {
  try {
    mks::Logger::Infof("Begin Pong test.");

    a.init();

    mks::Lua l{};
    lua_register(l.L, "LoadSoundEffect", lua_LoadSoundEffect);
    lua_register(l.L, "PlaySoundEffect", lua_PlaySoundEffect);
    lua_register(l.L, "GetGamepadInput", lua_GetGamepadInput);
    lua_register(l.L, "LoadTexture", lua_LoadTexture);
    lua_register(l.L, "AdjustVBO", lua_AdjustVBO);

    mks::Gamepad::Enable();

    auto w = mks::Window{};
    ww = &w;
    w.Begin("Pong", 1024, 768);
    w.v.AssertDriverValidationLayersSupported();
    w.v.AssertDriverExtensionsSupported(w.requiredExtensionNames);
    w.v.CreateInstance("Pong_test", 1, 0, 0);
    w.v.UsePhysicalDevice(0);
    w.Bind();
    auto b = w.GetDrawableAreaExtentBounds();
    w.v.width = b.width;
    w.v.height = b.height;

    auto gamePad1 = mks::Gamepad{0};
    mks::Logger::Infof("Controller Id: %d, Name: %s", gamePad1.index, gamePad1.GetControllerName());
    gamePad1.Open();

    if (!l.ReloadScript("../assets/lua/pong.lua")) {
      throw mks::Logger::Errorf(l.GetError());
    }

    w.v.InitSwapChain();
    w.v.SetVertexBufferData(vertices, indices);

    w.v.CreateImageViews();           // pre
    w.v.CreateRenderPass();           // pre
    w.v.CreateDescriptorSetLayout();  // pre
    w.v.CreateGraphicsPipeline();     // pre
    w.v.CreateFrameBuffers();         // pre
    w.v.CreateCommandPool();          // setting

    w.v.CreateTextureImage(textureFiles[0].c_str());  // cmd
    w.v.CreateTextureImageView();                     // setting

    w.v.CreateTextureSampler();  // setting

    w.v.CreateVertexBuffer();                         // cmd
    w.v.CreateIndexBuffer();                          // cmd
    w.v.CreateUniformBuffers(sizeof(ubo_MVPMatrix));  // cmd

    w.v.CreateDescriptorPool();  // setting
    w.v.CreateDescriptorSets();  // setting
    w.v.CreateCommandBuffers();  // these theoretically would get used in render loop by me
    w.v.CreateSyncObjects();     // fence and semaphores

    ubo_MVPMatrix ubo{};  // model_view_projection_matrix

    w.RenderLoop(
        60,
        [&l](auto& e) { mks::Gamepad::OnInput(e); },
        [&w, &ubo, &l](float deltaTime) {
          lua_getglobal(l.L, "OnUpdate");
          lua_pushnumber(l.L, deltaTime);
          int result = lua_pcall(l.L, 1, 4, 0);
          float angle = lua_tonumber(l.L, -4);
          float x = lua_tonumber(l.L, -3);
          float y = lua_tonumber(l.L, -2);
          float z = lua_tonumber(l.L, -1);

          ubo.model =
              glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 0.0f, 1.0f));
          ubo.view = glm::lookAt(
              glm::vec3(x, y, z),
              glm::vec3(0.0f, 0.0f, 0.0f),
              glm::vec3(0.0f, 0.0f, 1.0f));
          ubo.proj = glm::perspective(
              glm::radians(45.0f),
              w.v.swapChainExtent.width / (float)w.v.swapChainExtent.height,
              0.1f,
              10.0f);
          ubo.proj[1][1] *= -1;

          if (isVBODirty) {
            isVBODirty = false;
            w.v.UpdateVertexBuffer();
          }
          w.v.UpdateUniformBuffer(w.v.currentFrame, &ubo);
        });

    w.v.DeviceWaitIdle();
    gamePad1.Close();
    w.v.Cleanup();
    w.End();

    mks::Logger::Infof("End of test.");
    return EXIT_SUCCESS;
  } catch (const std::runtime_error& e) {
    std::cerr << "Fatal: " << e.what() << std::endl;
    return EXIT_FAILURE;
  } catch (...) {
    std::cerr << "Unexpected error" << std::endl;
    return EXIT_FAILURE;
  }
}