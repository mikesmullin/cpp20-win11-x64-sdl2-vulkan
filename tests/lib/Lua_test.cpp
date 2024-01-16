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

int lua_HostFunction(lua_State* L) {
  float a = (float)lua_tonumber(L, 1);
  float b = (float)lua_tonumber(L, 2);
  std::cout << "[C++] HostFunction(" << a << ", " << b << ") called." << std::endl;
  float c = a * b;
  lua_pushnumber(L, c);
  return 1;
}

}  // namespace

int main() {
  const char* infile = "../assets/lua/test.lua";

  lua_State* L = luaL_newstate();
  luaL_openlibs(L);

  lua_register(L, "HostFunction", lua_HostFunction);

  int result = luaL_dofile(L, infile);
  if (result != LUA_OK) {
    PrintLuaError(L);
    return -1;
  }

  // lua_getglobal(L, "a");
  // if (lua_isnumber(L, -1)) {
  //   float a_in_cpp = (float)lua_tonumber(L, -1);
  //   std::cout << "a_in_cpp = " << a_in_cpp << std::endl;
  // }

  return 0;
}