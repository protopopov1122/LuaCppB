#ifndef LUACPPB_INVOKE_IMPL_CORO_H_
#define LUACPPB_INVOKE_IMPL_CORO_H_

#include "luacppb/Invoke/Coro.h"

namespace LuaCppB {

  template <typename ... A>
  Internal::LuaFunctionCallResult LuaCoroutine::operator()(A &&... args) const {
    std::vector<LuaValue> result;
    LuaError error = this->call<A...>(result, std::forward<A>(args)...);
    return Internal::LuaFunctionCallResult(result, error);
  }

  template <typename ... A>
  LuaError LuaCoroutine::call(std::vector<LuaValue> &result, A &&... args) const {
    Internal::LuaStack stack(this->thread.toState());
    Internal::LuaFunctionArgument<Internal::LuaNativeValue, A...>::push(thread.toState(), this->runtime, std::forward<A>(args)...);
    LuaStatusCode status = static_cast<LuaStatusCode>(lua_resume(thread.toState(), nullptr, sizeof...(args)));
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
      return LuaError(status, value.value_or(LuaValue()));
    }
  }
}

#endif