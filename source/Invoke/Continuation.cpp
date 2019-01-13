#include "luacppb/Invoke/Continuation.h"

namespace LuaCppB {

#ifdef LUACPPB_COROUTINE_SUPPORT

  LuaContinuation::LuaContinuation(const LuaReferenceHandle &handle, LuaCppRuntime &runtime)
    : handle(handle), runtime(runtime) {}
#endif
}