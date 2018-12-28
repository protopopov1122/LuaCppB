#ifndef LUACPPB_CORE_THROW_H_
#define LUACPPB_CORE_THROW_H_

#include "luacppb/Value/Native.h"

namespace LuaCppB {

  template <typename T>
  void LuaThrow(lua_State *state, LuaCppRuntime &runtime, T &&value) {
    Internal::LuaNativeValue::push(state, runtime, value);
    lua_error(state);
  }
}


#endif