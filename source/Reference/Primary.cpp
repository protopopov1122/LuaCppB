#include "luacppb/Reference/Primary.h"
#include "luacppb/Core/StackGuard.h"
#include "luacppb/Core/Stack.h"

namespace LuaCppB::Internal {

  LuaGlobalVariable::LuaGlobalVariable(LuaState &state, const std::string &name)
    : LuaReference(state), state(state.getState()), name(name) {}

  bool LuaGlobalVariable::putOnTop(std::function<void (lua_State *)> callback) {
    Internal::LuaStackGuard guard(this->state);
    lua_getglobal(this->state, this->name.c_str());
    auto canary = guard.canary();
    callback(this->state);
    canary.assume(0);
    lua_pop(this->state, 1);
    return true;
  }

  bool LuaGlobalVariable::setValue(std::function<void (lua_State *)> gen) {
    Internal::LuaStackGuard guard(this->state);
    auto canary = guard.canary();
    gen(this->state);
    canary.assume(1);
    lua_setglobal(this->state, this->name.c_str());
    return true;
  }

  LuaStackReference::LuaStackReference(LuaState &state, int index)
    : LuaReference(state), state(state.getState()), index(index) {}

  bool LuaStackReference::putOnTop(std::function<void (lua_State *)> callback) {
    Internal::LuaStack stack(this->state);
    Internal::LuaStackGuard guard(this->state);
    guard.assumeIndex(this->index);
    stack.copy(this->index);
    auto canary = guard.canary();
    callback(this->state);
    canary.assume();
    stack.pop();
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