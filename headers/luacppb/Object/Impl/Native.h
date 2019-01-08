#ifndef LUACPPB_OBJECT_IMPL_NATIVE_H_
#define LUACPPB_OBJECT_IMPL_NATIVE_H_

#include "luacppb/Object/Native.h"


namespace LuaCppB::Internal {

  template <typename T>
  typename std::enable_if<std::is_pointer<T>::value>::type LuaNativeObject::push(lua_State *state, LuaCppRuntime &runtime, T value) {
    using P = typename std::remove_pointer<T>::type;
    Internal::LuaStack stack(state);
    LuaCppObjectBoxerRegistry &boxer = runtime.getObjectBoxerRegistry();
    if (boxer.canWrap<P>()) {
      boxer.wrap(state, value);
    } else {
      stack.push();
    }
  }

  template <typename T>
  typename std::enable_if<!std::is_pointer<T>::value && !is_smart_pointer<T>::value>::type
    LuaNativeObject::push(lua_State *state, LuaCppRuntime &runtime, T &value) {
    Internal::LuaStack stack(state);
    LuaCppObjectBoxerRegistry &boxer = runtime.getObjectBoxerRegistry();
    if (boxer.canWrap<T>()) {
      boxer.wrap(state, &value);
    } else {
      stack.push();
    }
  }

  template <typename T>
  typename std::enable_if<is_instantiation<std::unique_ptr, T>::value>::type
    LuaNativeObject::push(lua_State *state, LuaCppRuntime &runtime, T &value) {
    using V = typename T::element_type;
    Internal::LuaStack stack(state);
    LuaCppObjectBoxerRegistry &boxer = runtime.getObjectBoxerRegistry();
    if (boxer.canWrap<V>()) {
      boxer.wrap(state, std::move(value));
    } else {
      stack.push();
    }
  }

  template <typename T>
  typename std::enable_if<is_instantiation<std::shared_ptr, T>::value>::type
    LuaNativeObject::push(lua_State *state, LuaCppRuntime &runtime, T &value) {
    using V = typename T::element_type;
    Internal::LuaStack stack(state);
    LuaCppObjectBoxerRegistry &boxer = runtime.getObjectBoxerRegistry();
    if (boxer.canWrap<V>()) {
      boxer.wrap(state, value);
    } else {
      stack.push();
    }
  }
}

#endif