#include "luacppb/Reference/Reference.h"
#include "luacppb/Core/StackGuard.h"

namespace LuaCppB {

  bool LuaGlobalVariable::putOnTop(std::function<void (lua_State *)> callback) {
    LuaStackGuard guard(this->state);
    lua_getglobal(this->state, this->name.c_str());
    auto canary = guard.canary();
    callback(this->state);
    canary.assume(0);
    lua_pop(this->state, 1);
    return true;
  }

  bool LuaGlobalVariable::setValue(std::function<void (lua_State *)> gen) {
    LuaStackGuard guard(this->state);
    auto canary = guard.canary();
    gen(this->state);
    canary.assume(1);
    lua_setglobal(this->state, this->name.c_str());
    return true;
  }

  bool LuaStackReference::putOnTop(std::function<void (lua_State *)> callback) {
    LuaStackGuard guard(this->state);
    guard.assumeIndex(this->index);
    lua_pushvalue(this->state, this->index);
    auto canary = guard.canary();
    callback(this->state);
    canary.assume();
    lua_pop(this->state, 1);
    return true;
  }

  bool LuaStackReference::setValue(std::function<void (lua_State *)> gen) {
    return false;
  }

  LuaRegistryReference::LuaRegistryReference(lua_State *state, LuaCppRuntime &runtime, int index)
    : LuaReference(runtime), handle(state, index) {
    if (state == nullptr) {
      throw LuaCppBError("Lua state can't be null", LuaCppBErrorCode::InvalidState);
    }
  }

  bool LuaRegistryReference::putOnTop(std::function<void (lua_State *)> callback) {
    return this->handle.get(callback);
  }

  bool LuaRegistryReference::setValue(std::function<void (lua_State *)> gen) {
    return this->handle.set(gen);
  }
}