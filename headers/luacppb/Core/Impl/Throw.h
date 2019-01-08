#ifndef LUACPPB_CORE_IMPL_THROW_H_
#define LUACPPB_CORE_IMPL_THROW_H_

#include "luacppb/Core/Throw.h"

#ifdef LUACPPB_ERROR_SUPPORT

namespace LuaCppB {

  template <typename S, typename T>
  void LuaThrow(S &env, T &&value) {
    Internal::LuaNativeValue::push(env.getState(), env, value);
    lua_error(env.getState());
  }
}

#endif

#endif