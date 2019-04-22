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

  namespace Internal {

    template <std::size_t I>
    void LuaFactoryTableSet<I>::set(LuaReferenceHandle &) {}

    template <std::size_t I, typename A, typename ... B>
    void LuaFactoryTableSet<I, A, B...>::set(LuaReferenceHandle &table, A &&value, B &&... other) {
      if constexpr (is_instantiation<LuaFactory::Entry, A>::value) {
        table[value.key] = value.value;
      } else {
        table[I] = value;
      }
      LuaFactoryTableSet<I + 1, B...>::set(table, std::forward<B>(other)...);
    }
  }

  template <typename K, typename V>
  LuaFactory::Entry<K, V>::Entry(K key, V value)
    : key(key), value(value) {}

  template <typename T, typename ... A>
  LuaReferenceHandle LuaFactory::newTable(T &env, A &&... values) {
    LuaTable table = LuaTable::create(env.getState());
    LuaReferenceHandle handle = table.ref(env);
    Internal::LuaFactoryTableSet<1, A...>::set(handle, std::forward<A>(values)...);
    return handle;
  }

  template <typename T, typename F>
  LuaReferenceHandle LuaFactory::newFunction(T &env, F && fn) {
    auto callable = NativeCallable(std::forward<F>(fn), env);
    Internal::LuaStack stack(env.getState());
    callable.push(env.getState());
    LuaReferenceHandle ref(env.getState(), std::make_unique<Internal::LuaRegistryReference>(env.getState(), env, -1));
    stack.pop();
    return ref;
  }

#ifdef LUACPPB_COROUTINE_SUPPORT
  template <typename T>
  LuaReferenceHandle LuaFactory::newThread(T &env, LuaReferenceHandle handle) {
    LuaCoroutine coro = handle;
    Internal::LuaStack stack(env.getState());
    coro.push(env.getState());
    LuaReferenceHandle ref(env.getState(), std::make_unique<Internal::LuaRegistryReference>(env.getState(), env, -1));
    stack.pop();
    return ref;
  }
#endif

  template <typename T, typename V>
  LuaValue LuaFactory::wrap(T &env, V &value) {
    Internal::LuaNativeValue::push(env.getState(), env, value);
    Internal::LuaStack stack(env.getState());
    LuaValue val = stack.get().value_or(LuaValue::Nil);
    stack.pop();
    return val;
  }

  template <typename T, typename V>
  LuaValue LuaFactory::wrap(T &env, V &&value) {
    return LuaFactory::wrap(env, value);
  }

  template <typename T>
  LuaReferenceHandle LuaFactory::mkref(T &env, LuaData &data) {
    Internal::LuaStack stack(env.getState());
    stack.push(data);
    LuaReferenceHandle ref(env.getState(), std::make_unique<Internal::LuaRegistryReference>(env.getState(), env, -1));
    stack.pop();
    return ref;
  }
}

#endif
