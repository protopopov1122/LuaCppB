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

namespace LuaCppB::Internal {

  LuaTableField::LuaTableField(LuaReferenceHandle ref, LuaCppRuntime &runtime, const std::string &name)
    : LuaReference(runtime), ref(ref), name(name) {}

  bool LuaTableField::putOnTop(std::function<void (lua_State *)> callback) {
    bool result = false;
    this->ref.getReference().putOnTop([&](lua_State *state) {
      Internal::LuaStackGuard guard(state);
      Internal::LuaStack stack(state);
      if (stack.is<LuaType::Table>()) {
        stack.getField(-1, this->name);
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
        gen(state);
        canary.assume(1);
        stack.setField(-2, this->name);
        result = true;
      }
    });
    return result;
  }

  LuaArrayField::LuaArrayField(LuaReferenceHandle ref, LuaCppRuntime &runtime, lua_Integer index)
    : LuaReference(runtime), ref(ref), index(index) {}

  bool LuaArrayField::putOnTop(std::function<void (lua_State *)> callback) {
    bool result = false;
    this->ref.getReference().putOnTop([&](lua_State *state) {
      Internal::LuaStackGuard guard(state);
      Internal::LuaStack stack(state);
      if (stack.is<LuaType::Table>()) {
        stack.getIndex(-1, this->index);
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

  bool LuaArrayField::setValue(std::function<void (lua_State *)> gen) {
    bool result = false;
    this->ref.getReference().putOnTop([&](lua_State *state) {
      Internal::LuaStackGuard guard(state);
      Internal::LuaStack stack(state);
      if (stack.is<LuaType::Table>()) {
        auto canary = guard.canary();
        gen(state);
        canary.assume(1);
        stack.setIndex(-2, this->index);
        result = true;
      }
    });
    return result;
  }
}
