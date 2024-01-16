#pragma once

extern "C" {
#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>
}

#include <string>

namespace mks {

class Lua {
 public:
  lua_State* L;
  Lua();
  ~Lua();
  std::string GetError();
  bool ReloadScript(const char* file);
};
}  // namespace mks