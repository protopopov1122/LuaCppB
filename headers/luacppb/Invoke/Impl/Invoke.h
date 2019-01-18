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

#ifndef LUACPPB_INVOKE_IMPL_INVOKE_H_
#define LUACPPB_INVOKE_IMPL_INVOKE_H_

#include "luacppb/Invoke/Invoke.h"

namespace LuaCppB::Internal {

  template <typename T, typename ... A>
  LuaFunctionCallResult LuaFunctionInvoke::invoke(T &ref, LuaCppRuntime &runtime, A &&... args) {
    std::vector<LuaValue> results;
    LuaError error;
    ref.putOnTop([&](lua_State *state) {
      Internal::LuaStack stack(state);
      if (stack.is<LuaType::Function>(-1)) {
        error = LuaFunctionCall::call<A...>(state, -1, runtime, results, std::forward<A>(args)...);
#ifdef LUACPPB_COROUTINE_SUPPORT
      } else if (stack.is<LuaType::Thread>(-1)) {
        LuaCoroutine coro(LuaThread(state, -1), runtime);
        error = coro.call(results, args...);
#endif
      }
    });
    return LuaFunctionCallResult(results, error);
  }

  template <typename T, typename ... A>
  void LuaFunctionInvoke::invokeK(T &ref, LuaCppRuntime &runtime, std::unique_ptr<LuaFunctionContinuation> cont, A &&... args) {
    ref.putOnTop([&](lua_State *state) {
      Internal::LuaStack stack(state);
      if (stack.is<LuaType::Function>(-1)) {
        LuaFunctionCall::callK<A...>(state, -1, runtime, std::move(cont), std::forward<A>(args)...);
#ifdef LUACPPB_COROUTINE_SUPPORT
      } else if (stack.is<LuaType::Thread>(-1)) {
        std::vector<LuaValue> results;
        LuaCoroutine coro(LuaThread(state, -1), runtime);
        LuaError error = coro.call(results, args...);
        cont->call(state, runtime, error.getStatus(), results);
#endif
      }
    });
  }

  template <typename ... A>
  void LuaFunctionInvoke::yieldK(lua_State *state, LuaCppRuntime &runtime, std::unique_ptr<LuaFunctionContinuation> cont, A &&... args) {
    LuaFunctionCall::yieldK<A...>(state, runtime, std::move(cont), std::forward<A>(args)...);
  }
}

#endif
