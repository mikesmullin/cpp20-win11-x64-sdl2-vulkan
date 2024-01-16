#include "../../src/lib/Lua.hpp"

#include <iostream>
#include <vector>

#include "../../src/lib/Logger.hpp"

namespace {

uint8_t nextEid = 1;
struct Entity {
  uint8_t eid;
};

int lua_RegisterEntity(lua_State* L) {
  // TODO: heap alloc needs free
  Entity* e = new Entity{nextEid++};
  lua_pushinteger(L, e->eid);
  return 1;
}

struct Component {
  int OnUpdate;
};

std::vector<Component*> componentRegistry;

int lua_RegisterComponent(lua_State* L) {
  Component* c = new Component{};
  c->OnUpdate = luaL_ref(L, LUA_REGISTRYINDEX);
  componentRegistry.push_back(c);
  return 0;
}

}  // namespace

int main() {
  auto l = mks::Lua{};

  lua_register(l.L, "RegisterEntity", lua_RegisterEntity);
  lua_register(l.L, "RegisterComponent", lua_RegisterComponent);

  if (!l.ReloadScript("../assets/lua/test.lua")) {
    std::cerr << l.GetError() << std::endl;
    return -1;
  }

  for (Component* component : componentRegistry) {
    lua_rawgeti(l.L, LUA_REGISTRYINDEX, component->OnUpdate);
    if (lua_isfunction(l.L, -1)) {
      int result2 = lua_pcall(l.L, 0, 0, 0);
      if (result2 != LUA_OK) {
        std::cerr << l.GetError() << std::endl;
        return -1;
      }
      std::cout << "[C++] invoked callback. " << std::endl;
    }
  }

  return 0;
}