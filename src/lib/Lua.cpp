#include "Lua.hpp"

#include <string>

#include "Logger.hpp"

namespace mks {
Lua::Lua() {
  L = luaL_newstate();
  luaL_openlibs(L);
}
Lua::~Lua() {
}

std::string Lua::GetError() {
  return lua_tostring(L, -1);
}

bool Lua::ReloadScript(const char* file) {
  int result = luaL_dofile(L, file);
  return result == LUA_OK;
}

}  // namespace mks