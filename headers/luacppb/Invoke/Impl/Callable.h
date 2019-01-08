#ifndef LUACPPB_INVOKE_IMPL_CALLABLE_H_
#define LUACPPB_INVOKE_IMPL_CALLABLE_H_

#include "luacppb/Invoke/Callable.h"

namespace LuaCppB {

  template <typename F>
  auto NativeCallable(F &&func, LuaCppRuntime &runtime) -> decltype(Internal::NativeInvocable<Internal::LuaNativeValue>::create(func, runtime)) {
    return Internal::NativeInvocable<Internal::LuaNativeValue>::create(std::forward<F>(func), runtime);
  }

  template <typename C, typename M>
  auto NativeCallable(C &object, M method, LuaCppRuntime &runtime) -> decltype(Internal::NativeMethod<Internal::LuaNativeValue>::create(object, method, runtime)) {
    return Internal::NativeMethod<Internal::LuaNativeValue>::create(object, method, runtime);
  }
  
  template <typename C, typename M>
  auto NativeCallable(C *object, M method, LuaCppRuntime &runtime) -> decltype(Internal::NativeMethod<Internal::LuaNativeValue>::create(object, method, runtime)) {
    return Internal::NativeMethod<Internal::LuaNativeValue>::create(object, method, runtime);
  }
}

#endif