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

#include "luacppb/Core/StackGuard.h"
#include "luacppb/Core/Error.h"

namespace LuaCppB::Internal {

  LuaStackCanary::LuaStackCanary(lua_State *state)
    : state(state) {
      this->size = lua_gettop(state);
  }

  bool LuaStackCanary::check(int offset) const noexcept {
    return lua_gettop(this->state) == this->size + offset;
  }

  void LuaStackCanary::assume(int offset) const {
    const int assumed = this->size + offset;
    if (lua_gettop(this->state) != assumed) {
      throw LuaCppBError("Assumed stack size " + std::to_string(assumed), LuaCppBErrorCode::StackSizeMismatch);
    }
  }

  LuaStackGuard::LuaStackGuard(lua_State *state)
    : state(state) {}

  std::size_t LuaStackGuard::size() const noexcept {
    return static_cast<std::size_t>(lua_gettop(this->state));
  }

  bool LuaStackGuard::checkCapacity(std::size_t slots) const noexcept {
    return static_cast<bool>(lua_checkstack(this->state, slots));
  }

  void LuaStackGuard::assumeCapacity(std::size_t slots) const {
    if (!lua_checkstack(this->state, slots)) {
      throw LuaCppBError("Expected at least " + std::to_string(slots) + " stack slots");
    }
  }

  bool LuaStackGuard::checkIndex(int index) const noexcept {
    return index <= lua_gettop(this->state) && index >= -lua_gettop(this->state);
  }

  void LuaStackGuard::assumeIndex(int index) const {
    if (!(index <= lua_gettop(this->state) && index >= -lua_gettop(this->state))) {
      throw LuaCppBError("Assumed stack index " + std::to_string(index) + " to be valid");
    }
  }

  LuaStackCanary LuaStackGuard::canary() const noexcept {
    return LuaStackCanary(this->state);
  }

  LuaStackCleaner::LuaStackCleaner(lua_State *state)
    : state(state), top(lua_gettop(state)) {}
  
  LuaStackCleaner::~LuaStackCleaner() {
    int currentTop = lua_gettop(this->state);
    if (currentTop > this->top) {
      lua_pop(this->state, currentTop - this->top);
    }
  }

  int LuaStackCleaner::getDelta() {
    return lua_gettop(this->state) - this->top;
  }
}
