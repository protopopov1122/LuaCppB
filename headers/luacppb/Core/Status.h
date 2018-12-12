#ifndef LUACPPB_STATUS_H_
#define LUACPPB_STATUS_H_

#include "luacppb/Base.h"

namespace LuaCppB {

  enum class LuaStatusCode {
    Ok = LUA_OK,
    RuntimeError = LUA_ERRRUN,
    MemAllocError = LUA_ERRMEM,
    MsgHandlerError = LUA_ERRERR,
    GCMethodError = LUA_ERRGCMM
  };
}

#endif