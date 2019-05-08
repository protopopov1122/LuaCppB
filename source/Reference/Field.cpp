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

#include "luacppb/Reference/Field.h"
#include "luacppb/Core/StackGuard.h"
#include "luacppb/Core/Impl/Stack.h"

namespace LuaCppB::Internal {
  
  LuaTableField::LuaTableField(LuaReferenceHandle ref, LuaCppRuntime &runtime, LuaValue value)
    : LuaReference(runtime), ref(std::move(ref)), index(std::move(value)) {}

  bool LuaTableField::putOnTop(std::function<void (lua_State *)> callback) {
    bool result = false;
    this->ref.getReference().putOnTop([&](lua_State *state) {
      Internal::LuaStackGuard guard(state);
      Internal::LuaStack stack(state);
      if (stack.is<LuaType::Table>()) {
        this->index.push(state);
        stack.getField(-2);
        if (!stack.is<LuaType::None>()) {
          auto canary = guard.canary();
          callback(state);
          canary.assume();
          result = true;
        }
        stack.pop(1);
      }
    });
    return result;
  }

  bool LuaTableField::setValue(std::function<void (lua_State *)> gen) {
    bool result = false;
    this->ref.getReference().putOnTop([&](lua_State *state) {
      Internal::LuaStackGuard guard(state);
      Internal::LuaStack stack(state);
      if (stack.is<LuaType::Table>()) {
        auto canary = guard.canary();
        this->index.push(state);
        gen(state);
        canary.assume(2);
        stack.setField(-3);
        result = true;
      }
    });
    return result;
  }

	lua_State *LuaTableField::putOnTop() {
    lua_State *state = this->ref.getReference().putOnTop();
    if (state == nullptr) {
      return nullptr;
    }
    Internal::LuaStack stack(state);
    if (stack.is<LuaType::Table>()) {
      this->index.push(state);
      stack.getField(-2);
      lua_remove(state, -2);
      return state;
    } else {
      lua_pop(state, 1);
      return nullptr;
    }
  }

  std::unique_ptr<LuaReference> LuaTableField::clone(LuaCppRuntime &runtime) {
    return std::make_unique<LuaTableField>(this->ref, runtime, this->index);
  }

  LuaStringField::LuaStringField(LuaReferenceHandle ref, LuaCppRuntime &runtime, const std::string &index)
    : LuaReference(runtime), ref(std::move(ref)), index(index) {}

  bool LuaStringField::putOnTop(std::function<void (lua_State *)> callback) {
    bool result = false;
    this->ref.getReference().putOnTop([&](lua_State *state) {
      Internal::LuaStackGuard guard(state);
      Internal::LuaStack stack(state);
      if (stack.is<LuaType::Table>()) {
        lua_pushstring(state, this->index.c_str());
        stack.getField(-2);
        if (!stack.is<LuaType::None>()) {
          auto canary = guard.canary();
          callback(state);
          canary.assume();
          result = true;
        }
        stack.pop(1);
      }
    });
    return result;
  }

  bool LuaStringField::setValue(std::function<void (lua_State *)> gen) {
    bool result = false;
    this->ref.getReference().putOnTop([&](lua_State *state) {
      Internal::LuaStackGuard guard(state);
      Internal::LuaStack stack(state);
      if (stack.is<LuaType::Table>()) {
        auto canary = guard.canary();
        lua_pushstring(state, this->index.c_str());
        gen(state);
        canary.assume(2);
        stack.setField(-3);
        result = true;
      }
    });
    return result;
  }

	lua_State *LuaStringField::putOnTop() {
    lua_State *state = this->ref.getReference().putOnTop();
    if (state == nullptr) {
      return nullptr;
    }
    if (lua_istable(state, -1)) {
      lua_getfield(state, -1, this->index.c_str());
      lua_remove(state, -2);
      return state;
    } else {
      lua_pop(state, 1);
      return nullptr;
    }
  }

  std::unique_ptr<LuaReference> LuaStringField::clone(LuaCppRuntime &runtime) {
    return std::make_unique<LuaStringField>(this->ref, runtime, this->index);
  }
}
