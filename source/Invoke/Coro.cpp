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

#include "luacppb/Invoke/Coro.h"

#ifdef LUACPPB_COROUTINE_SUPPORT

namespace LuaCppB {

  LuaCoroutine::LuaCoroutine(lua_State *state, int index, LuaCppRuntime &runtime)
    : thread(LuaThread::create(state)), runtime(runtime) {
    Internal::LuaStack stack(state);
    stack.copy(index);
    stack.move(thread.toState(), 1);
  }

  LuaCoroutine::LuaCoroutine(LuaThread &thread, LuaCppRuntime &runtime)
    : thread(thread), runtime(runtime) {}
  
  LuaCoroutine::LuaCoroutine(LuaThread &&thread, LuaCppRuntime &runtime)
    : thread(thread), runtime(runtime) {}

  LuaCoroutine::LuaCoroutine(LuaCppRuntime &runtime)
    : thread(), runtime(runtime) {}

  LuaCoroutine::LuaCoroutine(const LuaCoroutine &coro)
    : thread(coro.thread), runtime(coro.runtime) {}
  
  LuaCoroutine &LuaCoroutine::operator=(const LuaCoroutine &coro) {
    this->thread = coro.thread;
    this->runtime = coro.runtime;
    return *this;
  }

  LuaStatusCode LuaCoroutine::getStatus() const {
    return static_cast<LuaStatusCode>(lua_status(this->thread.toState()));
  }

  void LuaCoroutine::push(lua_State *state) const {
    thread.push(state);
  }
}

#endif
