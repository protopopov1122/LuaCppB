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

#include "luacppb/Core/Panic.h"
#include "luacppb/Core/Stack.h"

namespace LuaCppB::Internal {

  LuaPanicDispatcher LuaPanicDispatcher::singleton;

  lua_CFunction LuaPanicDispatcher::attach(lua_State *state, Handler handler) {
    this->handlers[state] = handler;
    return lua_atpanic(state, &LuaPanicDispatcher::atpanic);
  }

  void LuaPanicDispatcher::detach(lua_State *state) {
    if (this->handlers.count(state)) {
      this->handlers.erase(state);
    }
  }

  LuaPanicDispatcher &LuaPanicDispatcher::getGlobal() {
    return LuaPanicDispatcher::singleton;
  }

  int LuaPanicDispatcher::atpanic(lua_State *state) {
      LuaStack stack(state);
      if (stack.check(1)) {
#ifndef LUACPPB_INTERNAL_EMULATED_MAINTHREAD
        stack.getIndex<true>(LUA_REGISTRYINDEX, LUA_RIDX_MAINTHREAD);
#else
        stack.push(std::string(LUACPPB_RIDX_MAINTHREAD));
        stack.getField<true>(LUA_REGISTRYINDEX);
#endif
      state = stack.toThread();
      stack.pop();
    }
    auto &dispatcher = LuaPanicDispatcher::getGlobal();
    if (dispatcher.handlers.count(state)) {
      return dispatcher.handlers[state](state);
    } else {
      return 0;
    }
  }

  LuaPanicUnbind::LuaPanicUnbind(LuaPanicDispatcher &dispatcher)
    : dispatcher(dispatcher), state(nullptr) {}
  
  LuaPanicUnbind::LuaPanicUnbind(LuaPanicDispatcher &dispatcher, lua_State *state)
    : dispatcher(dispatcher), state(state) {}
    
  LuaPanicUnbind::LuaPanicUnbind(LuaPanicUnbind &&unbind)
    : dispatcher(unbind.dispatcher), state(unbind.state) {
    unbind.state = nullptr;
  }
  
  LuaPanicUnbind &LuaPanicUnbind::operator=(LuaPanicUnbind &&unbind) {
    this->state = unbind.state;
    unbind.state = nullptr;
    return *this;
  }

  LuaPanicUnbind::~LuaPanicUnbind() {
    if (this->state) {
      this->dispatcher.detach(this->state);
    }
  }
}