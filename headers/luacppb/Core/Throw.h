#ifndef LUACPPB_CORE_THROW_H_
#define LUACPPB_CORE_THROW_H_

#include "luacppb/Value/Native.h"

#ifdef LUACPPB_ERROR_SUPPORT

namespace LuaCppB {

  template <typename S, typename T>
  void LuaThrow(S &, T &&);
}

#endif

#include "luacppb/Core/Impl/Throw.h"

#endif