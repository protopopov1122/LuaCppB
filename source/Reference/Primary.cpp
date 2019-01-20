/*
  SPDX short identifier: MIT

  Copyright 2018-2019 Jevgēnijs Protopopovs

  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"),
  to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
  and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
  IN THE SOFTWARE.
*/

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

  LuaRegistryReference::LuaRegistryReference(LuaCppRuntime &runtime, Internal::LuaSharedRegistryHandle &handle)
    : LuaReference(runtime), handle(handle) {}

  bool LuaRegistryReference::putOnTop(std::function<void (lua_State *)> callback) {
    return this->handle.get(callback);
  }

  bool LuaRegistryReference::setValue(std::function<void (lua_State *)> gen) {
    return this->handle.set(gen);
  }

  LuaValue LuaRegistryReference::toValue() {
    return LuaValue::fromRegistry(this->handle).value_or(LuaValue::Nil);
  }
}
