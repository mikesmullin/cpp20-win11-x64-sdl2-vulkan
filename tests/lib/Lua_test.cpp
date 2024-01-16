extern "C" {
#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>
}

#include <iostream>

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

}  // namespace

int main() {
  const char* infile = "../assets/lua/test.lua";

  lua_State* L = luaL_newstate();
  luaL_openlibs(L);

  lua_register(L, "RegisterEntity", lua_RegisterEntity);

  int result = luaL_dofile(L, infile);
  if (result != LUA_OK) {
    PrintLuaError(L);
    return -1;
  }

  lua_getglobal(L, "AddStuff");
  if (lua_isfunction(L, -1)) {
    lua_pushnumber(L, 3.5f);
    lua_pushnumber(L, 7.1f);

    int result2 = lua_pcall(L, 2, 1, 0);
    if (result2 != LUA_OK) {
      PrintLuaError(L);
      return -1;
    }
    float r = (float)lua_tonumber(L, -1);
    std::cout << "[C++] AddStuff() returned " << r << std::endl;
  }

  // lua_getglobal(L, "a");
  // if (lua_isnumber(L, -1)) {
  //   float a_in_cpp = (float)lua_tonumber(L, -1);
  //   std::cout << "a_in_cpp = " << a_in_cpp << std::endl;
  // }

  return 0;
}