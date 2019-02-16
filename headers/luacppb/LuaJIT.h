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

#ifndef LUACPPB_LUAJIT_H_
#define LUACPPB_LUAJIT_H_

#include "luacppb/Base.h"

#ifdef LUACPPB_HAS_JIT

namespace LuaCppB {

  enum class LuaJITFlag {
    Mask = LUAJIT_MODE_MASK,
    Off = LUAJIT_MODE_OFF,
    On = LUAJIT_MODE_ON,
    Flush = LUAJIT_MODE_FLUSH
  };

  enum class LuaJITMode {

    Engine = LUAJIT_MODE_ENGINE,
    Debug = LUAJIT_MODE_DEBUG,

    Func = LUAJIT_MODE_FUNC,
    AllFunc = LUAJIT_MODE_ALLFUNC,
    AllSubFunc = LUAJIT_MODE_ALLSUBFUNC,

    Trace = LUAJIT_MODE_TRACE,

    WrapFunc = LUAJIT_MODE_WRAPCFUNC,

    Max = LUAJIT_MODE_MAX
  };

  class LuaJIT {
   public:
    LuaJIT(lua_State *);

    bool setMode(LuaJITMode, LuaJITFlag, int = 0);
    int version();
   private:
    lua_State *state;
  };
}

#endif
#endif