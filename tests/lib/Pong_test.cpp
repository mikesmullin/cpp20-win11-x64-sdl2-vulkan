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

const char* WINDOW_TITLE = "Pong";
const u8 MAX_FPS = 60;

struct Mesh {
  glm::vec2 vertex;
};

struct Instance {
  glm::vec3 pos{0.0f, 0.0f, 0.0f};
  glm::vec3 rot{0.0f, 0.0f, 0.0f};
  glm::vec3 scale{1.0f, 1.0f, 1.0f};
  u32 texId{0};
};

struct World {
  glm::vec3 cam{0.0f, 0.0f, 0.0f};
  glm::vec3 look{0.0f, 0.0f, 0.0f};
  glm::vec2 user1{0.0f, 0.0f};
  glm::vec2 user2{0.0f, 0.0f};
  f32 aspect = 1.0f / 1;  // square
};

struct ubo_ProjView {
  glm::mat4 proj;
  glm::mat4 view;
  glm::vec2 user1;
  glm::vec2 user2;
};

std::vector<Mesh> vertices = {{{-0.5f, -0.5f}}, {{0.5f, -0.5f}}, {{0.5f, 0.5f}}, {{-0.5f, 0.5f}}};

std::vector<uint16_t> indices = {0, 1, 2, 2, 3, 0};

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

const f32 MAX_X = 1.0f;
const f32 MAX_Y = 1.0f;
const f32 MAX_Z = 10.0f;
const f32 MAX_SCALE = 0.4f;
const u32 MAX_INSTANCES = 255;  // TODO: find out how to exceed this limit
bool isVBODirty = true;
std::vector<Instance> instances(0);
int lua_AddInstance(lua_State* L) {
  const u8 id = instances.size();
  instances.push_back({});
  isVBODirty = true;
  lua_pushnumber(L, id);
  return 1;
}

mks::Audio a{};

int lua_LoadAudioFile(lua_State* L) {
  auto file = lua_tostring(L, 1);
  a.loadAudioFile(file);
  return 1;
}

int lua_PlayAudio(lua_State* L) {
  const int id = lua_tointeger(L, 1);
  const bool loop = lua_toboolean(L, 2);
  a.playAudio(id, loop);
  return 2;
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
  return 1;
}

std::vector<std::string> shaderFiles;
int lua_LoadShader(lua_State* L) {
  auto file = lua_tostring(L, 1);
  shaderFiles.push_back(static_cast<std::string>(file));
  return 1;
}

int lua_ReadInstanceVBO(lua_State* L) {
  const u8 id = lua_tointeger(L, 1);

  auto& instance = instances[id];

  lua_pushnumber(L, instance.pos.x);
  lua_pushnumber(L, instance.pos.y);
  lua_pushnumber(L, instance.pos.z);
  lua_pushnumber(L, instance.rot.x);
  lua_pushnumber(L, instance.rot.y);
  lua_pushnumber(L, instance.rot.z);
  lua_pushnumber(L, instance.scale.x);
  lua_pushnumber(L, instance.scale.y);
  lua_pushnumber(L, instance.scale.z);
  lua_pushnumber(L, instance.texId);

  return 11;
}

int lua_WriteInstanceVBO(lua_State* L) {
  const u8 id = lua_tointeger(L, 1);
  const f32 pos_x = lua_tonumber(L, 2);
  const f32 pos_y = lua_tonumber(L, 3);
  const f32 pos_z = lua_tonumber(L, 4);
  const f32 rot_x = lua_tonumber(L, 5);
  const f32 rot_y = lua_tonumber(L, 6);
  const f32 rot_z = lua_tonumber(L, 7);
  const f32 scale_x = lua_tonumber(L, 8);
  const f32 scale_y = lua_tonumber(L, 9);
  const f32 scale_z = lua_tonumber(L, 10);
  const u8 texId = lua_tointeger(L, 11);
  auto& instance = instances[id];
  instance.pos = glm::vec3(pos_x, pos_y, pos_z);
  instance.rot = glm::vec3(rot_x, rot_y, rot_z);
  instance.scale = glm::vec3(scale_x, scale_y, scale_z);
  instance.texId = texId;
  isVBODirty = true;
  return 11;
}

World world{{0.0f, 1.0f, 2.0f}, {0.0f, 0.0f, 0.0f}};
std::vector<bool> isUBODirty{true, true};
void markWorldDirty() {
  isUBODirty[0] = true;
  isUBODirty[1] = true;
}
int lua_WriteWorldUBO(lua_State* L) {
  const f32 aspect = lua_tonumber(L, 1);
  const f32 camX = lua_tonumber(L, 2);
  const f32 camY = lua_tonumber(L, 3);
  const f32 camZ = lua_tonumber(L, 4);
  const f32 lookX = lua_tonumber(L, 5);
  const f32 lookY = lua_tonumber(L, 6);
  const f32 lookZ = lua_tonumber(L, 7);
  const f32 user1X = lua_tonumber(L, 8);
  const f32 user1Y = lua_tonumber(L, 9);
  const f32 user2X = lua_tonumber(L, 10);
  const f32 user2Y = lua_tonumber(L, 11);
  world.aspect = aspect;
  world.cam = glm::vec3(camX, camY, camZ);
  world.look = glm::vec3(lookX, lookY, lookZ);
  world.user1 = glm::vec2(user1X, user1Y);
  world.user2 = glm::vec2(user2X, user2Y);
  markWorldDirty();
  return 11;
}

}  // namespace

int main(int argc, char* argv[]) {
  try {
    // test();
    mks::Logger::Infof("Begin %s test.", WINDOW_TITLE);

    srand((unsigned)time(NULL));  // use current time as random seed

    a.init();

    mks::Lua l{};
    lua_register(l.L, "LoadAudioFile", lua_LoadAudioFile);
    lua_register(l.L, "PlayAudio", lua_PlayAudio);
    lua_register(l.L, "GetGamepadInput", lua_GetGamepadInput);
    lua_register(l.L, "AddInstance", lua_AddInstance);
    lua_register(l.L, "LoadTexture", lua_LoadTexture);
    lua_register(l.L, "LoadShader", lua_LoadShader);
    lua_register(l.L, "ReadInstanceVBO", lua_ReadInstanceVBO);
    lua_register(l.L, "WriteInstanceVBO", lua_WriteInstanceVBO);
    lua_register(l.L, "WriteWorldUBO", lua_WriteWorldUBO);

    mks::Gamepad::Enable();

    auto w = mks::Window{};
    w.Begin(WINDOW_TITLE, 800, 800);
    // w.v.aspectRatio = 1.0f / 1;  // ASPECT_SQUARE (default)
    // w.v.aspectRatio = 16.0f / 9; // ASPECT_WIDESCREEN
    w.v.AssertDriverValidationLayersSupported();
    w.v.AssertDriverExtensionsSupported(w.requiredExtensionNames);
    char instance_name[255];
    std::sprintf(instance_name, "%s_test", WINDOW_TITLE);
    w.v.CreateInstance(instance_name, 1, 0, 0);
    w.v.UsePhysicalDevice(0);
    w.Bind();

    auto b = w.GetDrawableAreaExtentBounds();
    w.KeepAspectRatio(b.width, b.height);

    auto gamePad1 = mks::Gamepad{0};
    mks::Logger::Infof("Controller Id: %d, Name: %s", gamePad1.index, gamePad1.GetControllerName());
    gamePad1.Open();

    if (!l.ReloadScript("../assets/lua/pong.lua")) {
      throw mks::Logger::Errorf(l.GetError());
    }

    w.v.InitSwapChain();
    w.v.CreateImageViews();
    w.v.CreateRenderPass();
    w.v.CreateDescriptorSetLayout();  // takes user data inputs
    w.v.CreateGraphicsPipeline(       // reads shaders in
        shaderFiles[0],
        shaderFiles[1],
        sizeof(Mesh),
        sizeof(Instance),
        5,
        {0, 1, 1, 1, 1},
        {0, 1, 2, 3, 4},
        {/*VK_FORMAT_R32G32_SFLOAT*/ 103,
         /*VK_FORMAT_R32G32B32_SFLOAT*/ 106,
         /*VK_FORMAT_R32G32B32_SFLOAT*/ 106,
         /*VK_FORMAT_R32G32B32_SFLOAT*/ 106,
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
    w.v.CreateVertexBuffer(
        1,
        sizeof(Instance) * MAX_INSTANCES /*VectorSize(instances)*/,
        instances.data());
    w.v.CreateIndexBuffer(sizeof(indices[0]) * indices.size(), indices.data());
    w.v.CreateUniformBuffers(sizeof(ubo_ProjView));

    w.v.CreateDescriptorPool();  // setting
    w.v.CreateDescriptorSets();  // setting
    w.v.CreateCommandBuffers();  // these theoretically would get used in render loop by me
    w.v.CreateSyncObjects();     // fence and semaphores

    ubo_ProjView ubo1{};                                    // projection x view matrices
    w.v.drawIndexCount = static_cast<u32>(indices.size());  // vertices per mesh (two triangles)

    w.RenderLoop(
        MAX_FPS,
        [&l](auto& e) { mks::Gamepad::OnInput(e); },
        [&w, &ubo1, &l](float deltaTime) {
          lua_getglobal(l.L, "OnUpdate");
          lua_pushnumber(l.L, deltaTime);
          int result = lua_pcall(l.L, 1, 0, 0);

          if (isVBODirty) {
            isVBODirty = false;
            w.v.instanceCount = instances.size();
            w.v.UpdateVertexBuffer(1, VectorSize(instances), instances.data());
          }

          if (isUBODirty[w.v.currentFrame]) {
            isUBODirty[w.v.currentFrame] = false;
            ubo1.view = glm::lookAt(
                glm::vec3(world.cam.x, world.cam.y, world.cam.z),
                glm::vec3(world.look.x, world.look.y, world.look.z),
                glm::vec3(0.0f, -1.0f, 0.0f));
            // ubo1.view = glm::translate(ubo1.view, glm::vec3(x, y, z));
            w.v.aspectRatio = world.aspect;  // sync viewport
            ubo1.proj = glm::perspective(
                glm::radians(45.0f),
                // w.v.swapChainExtent.width / (float)w.v.swapChainExtent.height,
                world.aspect,
                0.1f,  // TODO: adjust clipping range for z depth?
                10.0f);
            ubo1.proj[1][1] *= -1;
            ubo1.user1 = world.user1;
            ubo1.user2 = world.user2;
            // TODO: not sure i make use of one UBO per frame, really
            w.v.UpdateUniformBuffer(w.v.currentFrame, &ubo1);
          }
        });

    w.v.DeviceWaitIdle();
    gamePad1.Close();
    w.v.Cleanup();
    a.shutdown();
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