#ifndef LUACPPB_INVOKE_CALLABLE_H_
#define LUACPPB_INVOKE_CALLABLE_H_

#include "luacppb/Invoke/Native.h"
#include "luacppb/Value/Native.h"

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

  Internal::LuaLambdaHandle<LuaReferenceHandle> LuaLambda(LuaReferenceHandle &&ref) {
    return Internal::LuaLambdaHandle<LuaReferenceHandle>(std::forward<LuaReferenceHandle>(ref));
  }
}

#endif