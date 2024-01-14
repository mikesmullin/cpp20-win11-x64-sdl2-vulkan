extern "C" {
#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>
}

#include <iostream>

bool CheckLua(lua_State* L, int r) {
  if (r != LUA_OK) {
    std::string errormsg = lua_tostring(L, -1);
    std::cout << errormsg << std::endl;
    return false;
  }
  return true;
}

int main() {
  const char* infile = "../assets/lua/test.lua";

  lua_State* L = luaL_newstate();
  luaL_openlibs(L);

  if (CheckLua(L, luaL_dofile(L, infile))) {
    lua_getglobal(L, "a");
    if (lua_isnumber(L, -1)) {
      float a_in_cpp = (float)lua_tonumber(L, -1);
      std::cout << "a_in_cpp = " << a_in_cpp << std::endl;
    }
  }

  return 0;
}