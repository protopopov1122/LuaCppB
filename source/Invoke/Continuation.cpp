#include "luacppb/Invoke/Continuation.h"

namespace LuaCppB {

  LuaContinuation::LuaContinuation(const LuaReferenceHandle &handle, LuaCppRuntime &runtime)
    : handle(handle), runtime(runtime) {}
}