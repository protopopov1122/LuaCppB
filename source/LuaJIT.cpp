#include "luacppb/LuaJIT.h"
#ifdef LUACPPB_HAS_JIT

namespace LuaCppB {

  LuaJIT::LuaJIT(lua_State *state) : state(state) {}

  bool LuaJIT::setMode(LuaJITMode mode, LuaJITFlag flag, int index) {
    return static_cast<bool>(luaJIT_setmode(this->state, index, static_cast<int>(mode) | static_cast<int>(flag)));
  }

  int LuaJIT::version() {
    return LUAJIT_VERSION_NUM;
  }
}

#endif