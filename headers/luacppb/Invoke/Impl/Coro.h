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

#ifndef LUACPPB_INVOKE_IMPL_CORO_H_
#define LUACPPB_INVOKE_IMPL_CORO_H_

#include "luacppb/Invoke/Coro.h"

#ifdef LUACPPB_COROUTINE_SUPPORT

namespace LuaCppB {

  template <typename ... A>
  Internal::LuaFunctionCallResult LuaCoroutine::operator()(A &&... args) const {
    std::vector<LuaValue> result;
    LuaError error = this->call<A...>(result, std::forward<A>(args)...);
    return Internal::LuaFunctionCallResult(result, std::move(error));
  }

  template <typename ... A>
  LuaError LuaCoroutine::call(std::vector<LuaValue> &result, A &&... args) const {
    Internal::LuaStack stack(this->thread.toState());
    Internal::LuaFunctionArgument<Internal::LuaNativeValue, A...>::push(thread.toState(), this->runtime.get(), std::forward<A>(args)...);
#ifndef LUACPPB_INTERNAL_COMPAT_501
    LuaStatusCode status = static_cast<LuaStatusCode>(lua_resume(thread.toState(), nullptr, sizeof...(args)));
#else
    LuaStatusCode status = static_cast<LuaStatusCode>(lua_resume(thread.toState(), sizeof...(args)));
#endif
    if (status == LuaStatusCode::Ok || status == LuaStatusCode::Yield) {
      int results = stack.getTop();
      while (results-- > 0) {
        result.push_back(LuaValue::peek(this->thread.toState()).value());
        stack.pop();
      }
      std::reverse(result.begin(), result.end());
      return LuaError(status);
    } else {
      std::optional<LuaValue> value = stack.get();
      stack.pop();
      return LuaError(status, value.value_or(LuaValue::Nil));
    }
  }
}

#endif

#endif
