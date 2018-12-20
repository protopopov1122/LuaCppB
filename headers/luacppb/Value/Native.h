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
    static typename std::enable_if<!LuaValue::is_constructible<T>() && !is_smart_pointer<T>::value>::type push(lua_State *state, LuaCppRuntime &runtime, T &value) {
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
    
    template <typename T>
    static typename std::enable_if<!LuaValue::is_constructible<T>() && is_instantiation<std::unique_ptr, T>::value>::type
      push(lua_State *state, LuaCppRuntime &runtime, T &value) {
      using V = typename T::element_type;
      LuaCppObjectBoxerRegistry &boxer = runtime.getObjectBoxerRegistry();
      if (boxer.canWrap<V>()) {
        boxer.wrap(state, std::move(value));
      }
    }
    
    template <typename T>
    static typename std::enable_if<!LuaValue::is_constructible<T>() && is_instantiation<std::shared_ptr, T>::value>::type
      push(lua_State *state, LuaCppRuntime &runtime, T &value) {
      using V = typename T::element_type;
      LuaCppObjectBoxerRegistry &boxer = runtime.getObjectBoxerRegistry();
      if (boxer.canWrap<V>()) {
        boxer.wrap(state, value);
      }
    }
  };
}

#endif