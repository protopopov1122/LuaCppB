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

#include "luacppb/Reference/Impl/FastPath.h"

#ifdef LUACPPB_FAST_REFERENCE_SUPPORT

namespace LuaCppB::Internal {
  LuaVariableRef::LuaVariableRef(LuaState &state, const std::string &name)
    : state(state), name(name) {}

  LuaVariableRef::LuaVariableRef(const LuaVariableRef &ref)
    : state(ref.state), name(ref.name) {}
  LuaVariableRef::LuaVariableRef(LuaVariableRef &&ref)
    : state(ref.state), name(std::move(ref.name)), handle(std::move(ref.handle)) {}

  LuaVariableRef &LuaVariableRef::operator=(const LuaVariableRef &ref) {
    this->name = ref.name;
    return *this;
  }

  LuaVariableRef &LuaVariableRef::operator=(LuaVariableRef &&ref) {
    this->name = std::move(ref.name);
    this->handle = std::move(ref.handle);
    return *this;
  }

  LuaReferenceHandle &LuaVariableRef::getHandle() const {
    LuaVariableRef &ref = *const_cast<LuaVariableRef *>(this);
    if (!this->handle.valid() && !this->name.empty()) {
      ref.handle = LuaReferenceHandle(this->state.getState(), std::make_unique<LuaGlobalVariable>(this->state, this->name));
    }
    return ref.handle;
  }

  LuaCppRuntime &LuaVariableRef::getRuntime() const {
    return this->state;
  }

  bool LuaVariableRef::valid() const {
    return this->state.isValid() && !this->name.empty();
  }

  bool LuaVariableRef::exists() const {
    if (this->valid()) {
      lua_State *state = this->state.getState();
      lua_getglobal(state, this->name.c_str());
      bool exists = !lua_isnoneornil(state, -1);
      lua_pop(state, 1);
      return exists;
    } else {
      return false;
    }
  }

  LuaType LuaVariableRef::getType() const {
    if (this->valid()) {
      lua_State *state = this->state.getState();
      lua_getglobal(state, this->name.c_str());
      LuaType type = static_cast<LuaType>(lua_type(state, -1));
      lua_pop(state, 1);
      return type;
    } else {
      return LuaType::None;
    }
  }

  LuaValue LuaVariableRef::operator*() const {
    if (this->valid()) {
      lua_State *state = this->state.getState();
      lua_getglobal(state, this->name.c_str());
      LuaValue value = LuaValue::peek(state, -1).value_or(LuaValue());
      lua_pop(state, 1);
      return value;
    } else {
      return LuaValue::Nil;
    }
  }

  bool operator==(const LuaVariableRef &ref, const LuaReferenceHandle &handle) {
    return ref.getHandle() == handle;
  }

  bool operator!=(const LuaVariableRef &ref, const LuaReferenceHandle &handle) {
    return ref.getHandle() != handle;
  }
}

#endif