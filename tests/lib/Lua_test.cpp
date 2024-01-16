extern "C" {
#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>
}

#include <iostream>
#include <vector>

namespace {
void PrintLuaError(lua_State* L) {
  std::string errormsg = lua_tostring(L, -1);
  std::cout << errormsg << std::endl;
}

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
  return 1;
}

}  // namespace

int main() {
  const char* infile = "../assets/lua/test.lua";

  lua_State* L = luaL_newstate();
  luaL_openlibs(L);

  lua_register(L, "RegisterEntity", lua_RegisterEntity);
  lua_register(L, "RegisterComponent", lua_RegisterComponent);

  int result = luaL_dofile(L, infile);
  if (result != LUA_OK) {
    PrintLuaError(L);
    return -1;
  }

  for (Component* component : componentRegistry) {
    lua_rawgeti(L, LUA_REGISTRYINDEX, component->OnUpdate);
    if (lua_isfunction(L, -1)) {
      int result2 = lua_pcall(L, 0, 0, 0);
      if (result2 != LUA_OK) {
        PrintLuaError(L);
        return -1;
      }
      std::cout << "[C++] invoked callback. " << std::endl;
    }
  }

  return 0;
}