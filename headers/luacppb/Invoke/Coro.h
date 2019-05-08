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

#ifndef LUACPPB_INVOKE_CORO_H_
#define LUACPPB_INVOKE_CORO_H_

#include "luacppb/Base.h"
#include "luacppb/Core/Status.h"
#include "luacppb/Core/Runtime.h"
#include "luacppb/Value/Native.h"
#include "luacppb/Invoke/Lua.h"

#ifdef LUACPPB_COROUTINE_SUPPORT

namespace LuaCppB {

  class LuaCoroutine : public LuaData {
   public:
    LuaCoroutine(lua_State *, int, LuaCppRuntime &);
    LuaCoroutine(LuaThread &, LuaCppRuntime &);
    LuaCoroutine(LuaThread &&, LuaCppRuntime &);
    LuaCoroutine(LuaCppRuntime &);
    LuaCoroutine(const LuaCoroutine &);
    LuaCoroutine &operator=(const LuaCoroutine &);
    LuaStatusCode getStatus() const;

    void push(lua_State *) const override;

    template <typename ... A>
    Internal::LuaFunctionCallResult operator()(A &&...) const;

    template <typename ... A>
    LuaError call(std::vector<LuaValue> &, A &&...) const;
   private:
    
    LuaThread thread;
    std::reference_wrapper<LuaCppRuntime> runtime;
  };
}

#endif

#endif
