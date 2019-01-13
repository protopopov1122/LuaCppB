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