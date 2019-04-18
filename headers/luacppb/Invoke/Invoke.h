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

#ifndef LUACPPB_INVOKE_INVOKE_H_
#define LUACPPB_INVOKE_INVOKE_H_

#include "luacppb/Reference/Base.h"
#include "luacppb/Invoke/Native.h"
#include "luacppb/Invoke/Lua.h"
#include "luacppb/Invoke/Coro.h"
#include <memory>

namespace LuaCppB::Internal {

  class LuaFunctionInvoke {
   public:
		template <typename T, typename ... A>
		static LuaFunctionCallResult invoke(T &, LuaCppRuntime &, A &&...);

#ifdef LUACPPB_COROUTINE_SUPPORT
		template <typename T, typename ... A>
		static void invokeK(T &, lua_State *, LuaCppRuntime &, std::unique_ptr<LuaFunctionContinuation>, A &&...);

		template <typename ... A>
		static void yieldK(lua_State *, LuaCppRuntime &, std::unique_ptr<LuaFunctionContinuation>, A &&...);
#endif
  };
}

#include "luacppb/Invoke/Impl/Invoke.h"

#endif
