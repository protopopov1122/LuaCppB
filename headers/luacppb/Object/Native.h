#ifndef LUACPPB_OBJECT_NATIVE_H_
#define LUACPPB_OBJECT_NATIVE_H_

#include "luacppb/Base.h"
#include "luacppb/Core/Runtime.h"
#include <type_traits>

namespace LuaCppB {

  class LuaNativeObject {
   public:
    template <typename T>
    static typename std::enable_if<std::is_pointer<T>::value>::type push(lua_State *state, LuaCppRuntime &runtime, T value) {
      LuaCppObjectBoxerRegistry &boxer = runtime.getObjectBoxerRegistry();
      using P = typename std::remove_pointer<T>::type;
      if (boxer.canWrap<P>()) {
        boxer.wrap(state, value);
      } else {
        lua_pushnil(state);
      }
    }

    template <typename T>
    static typename std::enable_if<!std::is_pointer<T>::value && !is_smart_pointer<T>::value>::type
      push(lua_State *state, LuaCppRuntime &runtime, T &value) {
      LuaCppObjectBoxerRegistry &boxer = runtime.getObjectBoxerRegistry();
      if (boxer.canWrap<T>()) {
        boxer.wrap(state, &value);
      } else {
        lua_pushnil(state);
      }
    }

    template <typename T>
    static typename std::enable_if<is_instantiation<std::unique_ptr, T>::value>::type
      push(lua_State *state, LuaCppRuntime &runtime, T &value) {
      using V = typename T::element_type;
      LuaCppObjectBoxerRegistry &boxer = runtime.getObjectBoxerRegistry();
      if (boxer.canWrap<V>()) {
        boxer.wrap(state, std::move(value));
      } else {
        lua_pushnil(state);
      }
    }

    template <typename T>
    static typename std::enable_if<is_instantiation<std::shared_ptr, T>::value>::type
      push(lua_State *state, LuaCppRuntime &runtime, T &value) {
      using V = typename T::element_type;
      LuaCppObjectBoxerRegistry &boxer = runtime.getObjectBoxerRegistry();
      if (boxer.canWrap<V>()) {
        boxer.wrap(state, value);
      } else {
        lua_pushnil(state);
      }
    }
  };
}

#endif