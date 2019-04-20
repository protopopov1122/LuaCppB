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

#include "luacppb/Reference/Registry.h"
#include "luacppb/Core/StackGuard.h"
#include "luacppb/Core/Stack.h"

namespace LuaCppB::Internal {

  void LuaRegistryHandle::push(lua_State *state) const {
    Internal::LuaStack stack(state);
    int ref = -1;
    this->get([&](lua_State *handleState) {
      Internal::LuaStack handleStack(handleState);
      handleStack.copy(-1);
      ref = handleStack.ref();
    });
    stack.getIndex<true>(LUA_REGISTRYINDEX, ref);
    stack.unref(ref);
  }

  LuaUniqueRegistryHandle::LuaUniqueRegistryHandle()
    : state(nullptr), ref(0) {}

  LuaUniqueRegistryHandle::LuaUniqueRegistryHandle(lua_State *state, int index)
    : state(nullptr), ref(0) {
    if (state) {
      Internal::LuaStack originalStack(state);
#ifndef LUACPPB_INTERNAL_EMULATED_MAINTHREAD
      originalStack.getIndex<true>(LUA_REGISTRYINDEX, LUA_RIDX_MAINTHREAD);
#else
      originalStack.push(std::string(LUACPPB_RIDX_MAINTHREAD));
      originalStack.getField<true>(LUA_REGISTRYINDEX);
#endif
      this->state = originalStack.toThread();
      originalStack.pop();
      Internal::LuaStack mainStack(this->state);

      originalStack.copy(index);
      if (state == this->state) {
        this->ref = mainStack.ref();
      } else {
        originalStack.move(this->state, 1);
        this->ref = mainStack.ref();
      }
    }
  }

  LuaUniqueRegistryHandle::LuaUniqueRegistryHandle(const LuaUniqueRegistryHandle &handle)
    : state(handle.state), ref(0) {
    if (this->state) {
      Internal::LuaStack stack(state);
      stack.getIndex<true>(LUA_REGISTRYINDEX, handle.ref);
      this->ref = stack.ref();
    }
  }

  LuaUniqueRegistryHandle::LuaUniqueRegistryHandle(LuaUniqueRegistryHandle &&handle)
    : state(handle.state), ref(handle.ref) {
    handle.state = nullptr;
    handle.ref = 0;
  }

  LuaUniqueRegistryHandle::~LuaUniqueRegistryHandle() {
    if (this->state) {
      Internal::LuaStack stack(this->state);
      stack.unref(this->ref);
    }
  }

  lua_State *LuaUniqueRegistryHandle::getState() {
    return this->state;
  }

  bool LuaUniqueRegistryHandle::hasValue() const {
    return this->state != nullptr;
  }

  bool LuaUniqueRegistryHandle::get(std::function<void (lua_State *)> callback) const {
    if (this->state) {
      Internal::LuaStackGuard guard(this->state);
      Internal::LuaStack stack(this->state);
      stack.getIndex<true>(LUA_REGISTRYINDEX, this->ref);
      auto canary = guard.canary();
      callback(this->state);
      canary.assume();
      lua_pop(this->state, 1);
    }
    return this->state != nullptr;
  }

  bool LuaUniqueRegistryHandle::set(std::function<void (lua_State *)> gen) {
    if (this->state) {
      Internal::LuaStackGuard guard(this->state);
      Internal::LuaStack stack(this->state);
      stack.unref(this->ref);
      auto canary = guard.canary();
      gen(this->state);
      canary.assume(1);
      this->ref = stack.ref();
    }
    return this->state != nullptr;
  }

  LuaSharedRegistryHandle::LuaSharedRegistryHandle()
    : handle(nullptr) {}
  
  LuaSharedRegistryHandle::LuaSharedRegistryHandle(lua_State *state, int index) {
    this->handle = std::make_shared<LuaUniqueRegistryHandle>(state, index);
  }

  LuaSharedRegistryHandle::LuaSharedRegistryHandle(const LuaSharedRegistryHandle &handle)
    : handle(handle.handle) {}
  
  LuaSharedRegistryHandle::LuaSharedRegistryHandle(const LuaRegistryHandle &handle) {
    handle.get([&](lua_State *state) {
      this->handle = std::make_shared<LuaUniqueRegistryHandle>(state, -1);
    });
  }

  LuaSharedRegistryHandle &LuaSharedRegistryHandle::operator=(const LuaSharedRegistryHandle &handle) {
    this->handle = handle.handle;
    return *this;
  }

  lua_State *LuaSharedRegistryHandle::getState() {
    return this->handle != nullptr ? this->handle->getState() : nullptr;
  }

  bool LuaSharedRegistryHandle::hasValue() const {
    return this->handle != nullptr && this->handle->hasValue();
  }

  bool LuaSharedRegistryHandle::get(std::function<void (lua_State *)> callback) const {
    if (this->handle) {
      return this->handle->get(callback);
    } else {
      return false;
    }
  }

  bool LuaSharedRegistryHandle::set(std::function<void (lua_State *)> gen) {
    if (this->handle) {
      return this->handle->set(gen);
    } else {
      return false;
    }
  }
}
