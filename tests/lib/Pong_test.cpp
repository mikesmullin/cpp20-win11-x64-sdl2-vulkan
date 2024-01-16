#include <functional>
#include <iostream>
#include <stdexcept>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "../../src/lib/Audio.hpp"
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

mks::Audio a{};

int lua_AddSoundEffect(lua_State* L) {
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

}  // namespace

int main() {
  try {
    mks::Logger::Infof("Begin Pong test.");

    a.init();

    auto l = mks::Lua{};
    lua_register(l.L, "AddSoundEffect", lua_AddSoundEffect);
    lua_register(l.L, "PlaySoundEffect", lua_PlaySoundEffect);
    lua_register(l.L, "GetGamepadInput", lua_GetGamepadInput);

    mks::Gamepad::Enable();

    auto w = mks::Window{};
    w.Begin("Pong_test", "Pong", 1024, 768);

    auto gamePad1 = mks::Gamepad{0};
    mks::Logger::Infof("Controller Id: %d, Name: %s", gamePad1.index, gamePad1.GetControllerName());
    gamePad1.Open();

    w.v.CreateImageViews();                           // pre
    w.v.CreateRenderPass();                           // pre
    w.v.CreateDescriptorSetLayout();                  // pre
    w.v.CreateGraphicsPipeline();                     // pre
    w.v.CreateFrameBuffers();                         // pre
    w.v.CreateCommandPool();                          // setting
    w.v.CreateTextureImage();                         // cmd
    w.v.CreateTextureImageView();                     // setting
    w.v.CreateTextureSampler();                       // setting
    w.v.CreateVertexBuffer();                         // cmd
    w.v.CreateIndexBuffer();                          // cmd
    w.v.CreateUniformBuffers(sizeof(ubo_MVPMatrix));  // cmd
    w.v.CreateDescriptorPool();                       // setting
    w.v.CreateDescriptorSets();                       // setting
    w.v.CreateCommandBuffers();  // these theoretically would get used in render loop by me
    w.v.CreateSyncObjects();     // fence and semaphores

    ubo_MVPMatrix ubo{};  // model_view_projection_matrix

    if (!l.ReloadScript("../assets/lua/pong.lua")) {
      throw mks::Logger::Errorf(l.GetError());
    }

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

          mks::Logger::Debugf("x %7.3f y %7.3f z %7.3f", x, y, z);

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