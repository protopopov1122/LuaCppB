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

#ifndef LUACPPB_CORE_STACKGUARD_H_
#define LUACPPB_CORE_STACKGUARD_H_

#include "luacppb/Base.h"
#include <cinttypes>

namespace LuaCppB::Internal {

  class LuaStackCanary {
   public:
    LuaStackCanary(lua_State *);

    bool check(int = 0) const noexcept;
    void assume(int = 0) const;
   private:
    lua_State *state;
    int size;
  };

  class LuaStackGuard {
   public:
    LuaStackGuard(lua_State *);

    std::size_t size() const noexcept;
    bool checkCapacity(std::size_t) const noexcept;
    void assumeCapacity(std::size_t) const;
    bool checkIndex(int) const noexcept;
    void assumeIndex(int) const;
    LuaStackCanary canary() const noexcept;
   private:
    lua_State *state;
  };

  class LuaStackCleaner {
   public:
    LuaStackCleaner(lua_State *);
    ~LuaStackCleaner();

    int getDelta();
   private:
    lua_State *state;
    int top;
  };
}

#endif
