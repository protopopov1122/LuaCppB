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
    : state(state), ref(std::move(ref)) {}

  LuaReferenceHandle::LuaReferenceHandle(const LuaReferenceHandle &handle) : state(handle.state) {
    handle.getReference().putOnTop([&](lua_State *state) {
      this->ref = std::make_unique<Internal::LuaRegistryReference>(state, handle.getRuntime(), -1);
    });
  }

  LuaReferenceHandle::LuaReferenceHandle(LuaReferenceHandle &&handle)
    : state(handle.state), ref(std::move(handle.ref)) {
    handle.state = nullptr;
    handle.ref = nullptr;
  }

  Internal::LuaReference &LuaReferenceHandle::getReference() const {
    return *this->ref;
  }

  LuaCppRuntime &LuaReferenceHandle::getRuntime() const {
    return this->ref->getRuntime();
  }

  LuaReferenceHandle LuaReferenceHandle::operator[](const std::string &name) {
    if (this->ref) {
      return LuaReferenceHandle(this->state, std::make_unique<Internal::LuaTableField>(*this, this->ref->getRuntime(), name));
    } else {
      return LuaReferenceHandle();
    }
  }

  LuaReferenceHandle LuaReferenceHandle::operator[](const char *name)  {
    return this->operator[](std::string(name));
  }

  LuaReferenceHandle LuaReferenceHandle::operator[](lua_Integer index) {
    if (this->ref) {
      return LuaReferenceHandle(this->state, std::make_unique<Internal::LuaArrayField>(*this, this->ref->getRuntime(), index));
    } else {
      return LuaReferenceHandle();
    }
  }

  LuaValue LuaReferenceHandle::operator*() const {
    return this->get<LuaValue>();
  }

  bool LuaReferenceHandle::isValid() const {
    return this->state != nullptr && this->ref != nullptr;
  }

  bool LuaReferenceHandle::exists() {
    bool exists = false;
    if (this->ref) {
      this->ref->putOnTop([&](lua_State *state) {
        Internal::LuaStack stack(state);
        exists = !(stack.is<LuaType::None>() || stack.is<LuaType::Nil>());
      });
    }
    return exists;
  }

  LuaType LuaReferenceHandle::getType() {
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
    if (handle.isValid()) {
      handle.getReference().putOnTop([&](lua_State *state) {
        this->ref = std::make_unique<Internal::LuaRegistryReference>(state, handle.getRuntime(), -1);
      });
    } else {
      this->ref = nullptr;
    }
    return *this;
  }

  LuaReferenceHandle LuaReferenceHandle::getMetatable() {
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
    if (this->isValid() && handle.isValid()) {
      LuaValue value = *handle;
      bool result = false;
      this->getReference().putOnTop([&](lua_State *state) {
        value.push(state);
        result = static_cast<bool>(lua_compare(state, -1, -2, LUA_OPEQ));
        lua_pop(state, 1);
      });
      return result;
    } else {
      return this->state == handle.state;
    }
  }

  bool LuaReferenceHandle::operator!=(const LuaReferenceHandle &handle) const {
    return !this->operator==(handle);
  }

  TableIterator LuaReferenceHandle::begin() {
    return TableIterator(this->state, *this);
  }

  const TableIterator &LuaReferenceHandle::end() const {
    return TableIterator::End;
  }
}
