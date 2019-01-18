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

#ifndef LUACPPB_VALUE_IMPL_FACTORY_H_
#define LUACPPB_VALUE_IMPL_FACTORY_H_

#include "luacppb/Value/Factory.h"

namespace LuaCppB {

  template <typename T>
  LuaReferenceHandle LuaValueFactory::newTable(T &env) {
    LuaTable table = LuaTable::create(env.getState());
    return table.ref(env);
  }

  template <typename T, typename F>
  LuaReferenceHandle LuaValueFactory::newFunction(T &env, F && fn) {
    auto callable = NativeCallable(std::forward<F>(fn), env);
    Internal::LuaStack stack(env.getState());
    callable.push(env.getState());
    LuaReferenceHandle ref(env.getState(), std::make_unique<Internal::LuaRegistryReference>(env.getState(), env, -1));
    stack.pop();
    return ref;
  }

#ifdef LUACPPB_COROUTINE_SUPPORT
  template <typename T>
  LuaReferenceHandle LuaValueFactory::newThread(T &env, LuaReferenceHandle handle) {
    LuaCoroutine coro = handle;
    Internal::LuaStack stack(env.getState());
    coro.push(env.getState());
    LuaReferenceHandle ref(env.getState(), std::make_unique<Internal::LuaRegistryReference>(env.getState(), env, -1));
    stack.pop();
    return ref;
  }
#endif

  template <typename T, typename V>
  LuaValue LuaValueFactory::wrap(T &env, V &value) {
    Internal::LuaNativeValue::push(env.getState(), env, value);
    Internal::LuaStack stack(env.getState());
    LuaValue val = stack.get().value_or(LuaValue());
    stack.pop();
    return val;
  }

  template <typename T, typename V>
  LuaValue LuaValueFactory::wrap(T &env, V &&value) {
    return LuaValueFactory::wrap(env, value);
  }
}

#endif
