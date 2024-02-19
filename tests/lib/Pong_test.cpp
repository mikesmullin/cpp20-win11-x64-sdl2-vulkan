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

const u8 MAX_FPS = 60;

struct Mesh {
  glm::vec2 vertex;
};

struct Instance {
  glm::vec3 pos;
  glm::vec3 rot;
  f32 scale{0.0f};
  u32 texId{0};
};

struct ubo_ProjView {
  glm::mat4 proj;
  glm::mat4 view;
};

std::vector<Mesh> vertices = {{{-0.5f, -0.5f}}, {{0.5f, -0.5f}}, {{0.5f, 0.5f}}, {{-0.5f, 0.5f}}};

std::vector<uint16_t> indices = {0, 1, 2, 2, 3, 0};

std::vector<Instance> instances;

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

std::vector<bool> isUBODirty{true, true};

mks::Window* ww;
bool isVBODirty = true;
u32 SELECTED_INSTANCE = 0;
int lua_AdjustVBO(lua_State* L) {
  auto u = lua_tointeger(L, 1);
  auto v = lua_tointeger(L, 2);
  instances[SELECTED_INSTANCE].texId = (instances[SELECTED_INSTANCE].texId + 1) % 14;
  isVBODirty = true;
  return 0;
}

f32 random(f32 a, f32 b) {
  return a + (((f32)rand()) / (f32)RAND_MAX) * (b - a);
}

s32 srandom(s32 a, s32 b) {
  return a + (((s32)rand()) / (s32)RAND_MAX) * (b - a);
}

u32 urandom(u32 a, u32 b) {
  return a + ((rand() / (f32)RAND_MAX) * (b - a));
}

f32 mymap(u32 n, u32 x, f32 a, f32 b) {
  return a + (((f32)n) / (f32)x) * (b - a);
}

}  // namespace

int main() {
  try {
    // test();
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

    // generate a random set of instances
    instances.resize(255);
    const f32 MAX_X = 1.0f;
    const f32 MAX_Y = 1.0f;
    const f32 MAX_Z = 10.0f;
    const f32 MAX_SCALE = 0.4f;
    srand((unsigned)time(NULL));  // use current time as random seed
    SELECTED_INSTANCE = urandom(0, instances.size());
    mks::Logger::Debugf("selected instance: %u", SELECTED_INSTANCE);
    for (u8 i = 0; i < instances.size(); i++) {
      instances[i] = {
          // TODO: fix x coord sign is rendered inverted
          {random(-MAX_X, MAX_X),
           random(-MAX_Y, MAX_Y),
           mymap(i, instances.size(), -MAX_Z, MAX_Z) /*random(-MAX_Z, MAX_Z)*/},
          {0.0f, 0.0f, 0.0f},
          random(0.1f, MAX_SCALE),
          urandom(0, 14),
      };
    }

    w.v.InitSwapChain();
    w.v.CreateImageViews();
    w.v.CreateRenderPass();
    w.v.CreateDescriptorSetLayout();  // takes user data inputs
    w.v.CreateGraphicsPipeline(       // reads shaders in
        "../assets/shaders/simple_shader.frag.spv",
        "../assets/shaders/simple_shader.vert.spv",
        sizeof(Mesh),
        sizeof(Instance),
        5,
        {0, 1, 1, 1, 1},
        {0, 1, 2, 3, 4},
        {/*VK_FORMAT_R32G32_SFLOAT*/ 103,
         /*VK_FORMAT_R32G32B32_SFLOAT*/ 106,
         /*VK_FORMAT_R32G32B32_SFLOAT*/ 106,
         /*VK_FORMAT_R32_SFLOAT*/ 100,
         /*VK_FORMAT_R32_UINT*/ 98},
        {offsetof(Mesh, vertex),
         offsetof(Instance, pos),
         offsetof(Instance, rot),
         offsetof(Instance, scale),
         offsetof(Instance, texId)});
    w.v.CreateFrameBuffers();
    w.v.CreateCommandPool();

    w.v.CreateTextureImage(textureFiles[0].c_str());
    w.v.CreateTextureImageView();
    w.v.CreateTextureSampler();
    w.v.CreateVertexBuffer(0, VectorSize(vertices), vertices.data());
    w.v.CreateVertexBuffer(1, VectorSize(instances), instances.data());
    w.v.CreateIndexBuffer(sizeof(indices[0]) * indices.size(), indices.data());
    w.v.CreateUniformBuffers(sizeof(ubo_ProjView));

    w.v.CreateDescriptorPool();  // setting
    w.v.CreateDescriptorSets();  // setting
    w.v.CreateCommandBuffers();  // these theoretically would get used in render loop by me
    w.v.CreateSyncObjects();     // fence and semaphores

    ubo_ProjView ubo1{};  // projection x view matrices
    ubo1.view = glm::lookAt(
        glm::vec3(0.0f, 1.0f, 2.0f),
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 0.0f, 1.0f));
    w.v.drawIndexCount = static_cast<u32>(indices.size());  // vertices per mesh (two triangles)
    w.v.instanceCount = instances.size();

    w.RenderLoop(
        MAX_FPS,
        [&l](auto& e) { mks::Gamepad::OnInput(e); },
        [&w, &ubo1, &l](float deltaTime) {
          lua_getglobal(l.L, "OnUpdate");
          lua_pushnumber(l.L, deltaTime);
          int result = lua_pcall(l.L, 1, 4, 0);
          f32 angle = lua_tonumber(l.L, -4);
          f32 x = lua_tonumber(l.L, -3);
          f32 y = lua_tonumber(l.L, -2);
          f32 z = lua_tonumber(l.L, -1);

          if (isVBODirty) {
            isVBODirty = false;
            w.v.UpdateVertexBuffer(1, VectorSize(instances), instances.data());
          }

          // TODO: cache lua inputs so not always dirty on every frame
          // if (isUBODirty[w.v.currentFrame]) {
          //   isUBODirty[w.v.currentFrame] = false;
          ubo1.view = glm::translate(ubo1.view, glm::vec3(x, y, z));
          ubo1.proj = glm::perspective(
              glm::radians(45.0f),
              w.v.swapChainExtent.width / (float)w.v.swapChainExtent.height,
              0.1f,
              10.0f);
          ubo1.proj[1][1] *= -1;
          // TODO: not sure i make use of one UBO per frame, really
          w.v.UpdateUniformBuffer(w.v.currentFrame, &ubo1);
          // }
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
  } catch (const std::exception& e) {
    std::cerr << "Caught exception: " << e.what() << std::endl;
    return EXIT_FAILURE;
  } catch (...) {
    std::cerr << "Unidentifiable error" << std::endl;
    return EXIT_FAILURE;
  }
}