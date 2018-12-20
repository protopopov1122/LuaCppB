#ifndef LUACPPB_VALUE_NATIVE_H_
#define LUACPPB_VALUE_NATIVE_H_

#include "luacppb/Base.h"
#include "luacppb/Value/Value.h"
#include "luacppb/Core/Runtime.h"
#include <type_traits>

namespace LuaCppB {

  class LuaNativeValue {
   public:
    template <typename T>
    static typename std::enable_if<LuaValue::is_constructible<T>()>::type push(lua_State *state, LuaCppRuntime &runtime, T &value) {
      LuaValue::create<T>(value).push(state);
    }

    template <typename T>
    static typename std::enable_if<LuaValue::is_constructible<T>()>::type push(lua_State *state, LuaCppRuntime &runtime, T &&value) {
      LuaValue::create<T>(value).push(state);
    }

    template <typename T>
    static typename std::enable_if<!LuaValue::is_constructible<T>()>::type push(lua_State *state, LuaCppRuntime &runtime, T &value) {
      LuaCppObjectBoxerRegistry &boxer = runtime.getObjectBoxerRegistry();
      if constexpr (std::is_pointer<T>()) {
        using P = typename std::remove_pointer<T>::type;
        if (boxer.canWrap<P>()) {
          boxer.wrap(state, value);
        }
      } else {
        if (boxer.canWrap<T>()) {
          boxer.wrap(state, &value);
        }
      }
    }
  };
}

#endif