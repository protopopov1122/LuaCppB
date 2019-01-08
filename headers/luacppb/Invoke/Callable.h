#ifndef LUACPPB_INVOKE_CALLABLE_H_
#define LUACPPB_INVOKE_CALLABLE_H_

#include "luacppb/Invoke/Lua.h"
#include "luacppb/Reference/Handle.h"
#include "luacppb/Invoke/Native.h"
#include "luacppb/Value/Native.h"

namespace LuaCppB {

  template <typename F>
  auto NativeCallable(F &&, LuaCppRuntime &) -> decltype(Internal::NativeInvocable<Internal::LuaNativeValue>::create(std::declval<F>(), std::declval<LuaCppRuntime>()));

  template <typename C, typename M>
  auto NativeCallable(C &, M, LuaCppRuntime &) -> decltype(Internal::NativeMethod<Internal::LuaNativeValue>::create(std::declval<C>(), std::declval<M>(), std::declval<LuaCppRuntime>()));
  
  template <typename C, typename M>
  auto NativeCallable(C *, M, LuaCppRuntime &) -> decltype(Internal::NativeMethod<Internal::LuaNativeValue>::create(std::declval<C>(), std::declval<M>(), std::declval<LuaCppRuntime>()));

  Internal::LuaLambdaHandle<LuaReferenceHandle> LuaLambda(LuaReferenceHandle &&);
}

#include "luacppb/Invoke/Impl/Callable.h"

#endif