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

#include "luacppb/Reference/Handle.h"
#include "luacppb/Reference/Primary.h"
#include "luacppb/Reference/Field.h"
#include "luacppb/Value/Iterator.h"
#include "luacppb/Core/Stack.h"

namespace LuaCppB {

  LuaReferenceHandle::LuaReferenceHandle()
    : state(nullptr), ref(nullptr) {}
  
  LuaReferenceHandle::LuaReferenceHandle(lua_State *state, std::unique_ptr<Internal::LuaReference> ref)
    : state(state), ref(std::move(ref)) {
    if (this->state) {
      Internal::LuaStack stack(this->state);
#ifndef LUACPPB_EMULATED_MAINTHREAD
      stack.getIndex<true>(LUA_REGISTRYINDEX, LUA_RIDX_MAINTHREAD);
#else
      stack.push(std::string(LUACPPB_RIDX_MAINTHREAD));
      stack.getField<true>(LUA_REGISTRYINDEX);
#endif
      this->state = stack.toThread();
      stack.pop();
    }
  }

  LuaReferenceHandle::LuaReferenceHandle(const LuaReferenceHandle &handle) : state(handle.state) {
    this->state = handle.state;
    if (this->state) {
      handle.getReference().putOnTop([&](lua_State *state) {
        this->ref = std::make_unique<Internal::LuaRegistryReference>(state, handle.getRuntime(), -1);
      });
    } else {
      this->ref = nullptr;
    }
  }

  LuaReferenceHandle::LuaReferenceHandle(LuaReferenceHandle &&handle)
    : state(handle.state), ref(std::move(handle.ref)) {
    handle.state = nullptr;
    handle.ref = nullptr;
    if (this->state) {
      Internal::LuaStack stack(this->state);
#ifndef LUACPPB_EMULATED_MAINTHREAD
      stack.getIndex<true>(LUA_REGISTRYINDEX, LUA_RIDX_MAINTHREAD);
#else
      stack.push(std::string(LUACPPB_RIDX_MAINTHREAD));
      stack.getField<true>(LUA_REGISTRYINDEX);
#endif
      this->state = stack.toThread();
      stack.pop();
    }
  }

  Internal::LuaReference &LuaReferenceHandle::getReference() const {
    return *this->ref;
  }

  LuaCppRuntime &LuaReferenceHandle::getRuntime() const {
    return this->ref->getRuntime();
  }

  LuaValue LuaReferenceHandle::operator*() const {
    return this->get<LuaValue>();
  }

  bool LuaReferenceHandle::valid() const {
    return this->state != nullptr && this->ref != nullptr;
  }

  bool LuaReferenceHandle::exists() const {
    bool exists = false;
    if (this->ref) {
      this->ref->putOnTop([&](lua_State *state) {
        Internal::LuaStack stack(state);
        exists = !(stack.is<LuaType::None>() || stack.is<LuaType::Nil>());
      });
    }
    return exists;
  }

  LuaType LuaReferenceHandle::getType() const {
    LuaType type = LuaType::None;
    if (this->ref) {
      this->ref->putOnTop([&](lua_State *state) {
        type = static_cast<LuaType>(lua_type(state, -1));
      });
    }
    return type;
  }

  LuaReferenceHandle &LuaReferenceHandle::operator=(const LuaReferenceHandle &handle) {
    this->state = handle.state;
    if (handle.valid()) {
      handle.getReference().putOnTop([&](lua_State *state) {
        this->ref = std::make_unique<Internal::LuaRegistryReference>(state, handle.getRuntime(), -1);
      });
    } else {
      this->ref = nullptr;
    }
    return *this;
  }

  LuaReferenceHandle LuaReferenceHandle::getMetatable() const {
    LuaReferenceHandle handle;
    this->getReference().putOnTop([&](lua_State *state) {
      Internal::LuaStack stack(state);
      stack.getMetatable(-1);
      handle = LuaReferenceHandle(state, std::make_unique<Internal::LuaRegistryReference>(state, this->getRuntime(), -1));
      stack.pop(1);
    });
    return handle;
  }

  void LuaReferenceHandle::setMetatable(LuaData &data) {
    this->getReference().putOnTop([&](lua_State *state) {
      data.push(state);
      lua_setmetatable(state, -2);
    });
  }

  void LuaReferenceHandle::setMetatable(LuaData &&data) {
    this->setMetatable(data);
  }

  bool LuaReferenceHandle::operator==(const LuaReferenceHandle &handle) const {
    if (this->valid() && handle.valid()) {
      LuaValue value = *handle;
      bool result = false;
      this->getReference().putOnTop([&](lua_State *state) {
        Internal::LuaStack stack(state);
        stack.push(value);
        result = stack.compare(-1, -2);
        stack.pop();
      });
      return result;
    } else {
      return this->state == handle.state;
    }
  }

  bool LuaReferenceHandle::operator!=(const LuaReferenceHandle &handle) const {
    return !this->operator==(handle);
  }

  TableIterator LuaReferenceHandle::begin() const {
    return TableIterator(this->state, *this);
  }

  const TableIterator &LuaReferenceHandle::end() const {
    return TableIterator::End;
  }
}
