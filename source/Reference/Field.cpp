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
  
  LuaTableField::LuaTableField(LuaReferenceHandle ref, LuaCppRuntime &runtime, LuaValue value)
    : LuaReference(runtime), ref(std::move(ref)), name(std::move(value)) {}

  bool LuaTableField::putOnTop(std::function<void (lua_State *)> callback) {
    bool result = false;
    this->ref.getReference().putOnTop([&](lua_State *state) {
      Internal::LuaStackGuard guard(state);
      Internal::LuaStack stack(state);
      if (stack.is<LuaType::Table>()) {
        this->name.push(state);
        lua_gettable(state, -2);
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
        this->name.push(state);
        gen(state);
        canary.assume(2);
        lua_settable(state, -3);
        result = true;
      }
    });
    return result;
  }
}
