#ifndef LUACPPB_INVOKE_CALLABLE_H_
#define LUACPPB_INVOKE_CALLABLE_H_

#include "luacppb/Invoke/Native.h"
#include "luacppb/Value/Native.h"

namespace LuaCppB {

  template <typename F>
  auto NativeCallable(F func, LuaCppRuntime &runtime) -> decltype(NativeInvocable<LuaNativeValue>::create(func, runtime)) {
    return NativeInvocable<LuaNativeValue>::create(func, runtime);
  }

  template <typename C, typename M>
  auto NativeCallable(C object, M method, LuaCppRuntime &runtime) -> decltype(NativeMethod<LuaNativeValue>::create(object, method, runtime)) {
    return NativeMethod<LuaNativeValue>::create(object, method, runtime);
  }
}

#endif