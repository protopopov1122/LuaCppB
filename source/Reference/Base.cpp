#include "luacppb/Reference/Base.h"

namespace LuaCppB::Internal {

  LuaReference::LuaReference(LuaCppRuntime &runtime) : runtime(runtime) {}

  LuaCppRuntime &LuaReference::getRuntime() {
    return this->runtime;
  }
}