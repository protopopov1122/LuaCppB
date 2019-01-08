#ifndef LUACPPB_CONTAINER_IMPL_CONTAINER_H_
#define LUACPPB_CONTAINER_IMPL_CONTAINER_H_

#include "luacppb/Container/Container.h"

namespace LuaCppB::Internal {

  template <typename T, class P>
  typename std::enable_if<is_instantiation<std::vector, T>::value>::type
    LuaCppContainer::push(lua_State *state, LuaCppRuntime &runtime, T &value) {
    LuaCppVector<P>::push(state, runtime, value);
  }

  template <typename T, class P>
  typename std::enable_if<std::is_const<T>::value && is_instantiation<std::vector, typename std::remove_const<T>::type>::value>::type
    LuaCppContainer::push(lua_State *state, LuaCppRuntime &runtime, T &value) {
    LuaCppVector<P>::push(state, runtime, value);
  }

  template <typename T, class P>
  typename std::enable_if<is_smart_pointer<T>::value && is_instantiation<std::vector, typename T::element_type>::value>::type
    LuaCppContainer::push(lua_State *state, LuaCppRuntime &runtime, T &value) {
    LuaCppVector<P>::push(state, runtime, value);
  }
  
  template <typename T, class P>
  typename std::enable_if<is_instantiation<std::map, T>::value>::type
    LuaCppContainer::push(lua_State *state, LuaCppRuntime &runtime, T &value) {
    LuaCppMap<P>::push(state, runtime, value);
  }

  template <typename T, class P>
  typename std::enable_if<std::is_const<T>::value && is_instantiation<std::map, typename std::remove_const<T>::type>::value>::type
    LuaCppContainer::push(lua_State *state, LuaCppRuntime &runtime, T &value) {
    LuaCppMap<P>::push(state, runtime, value);
  }

  template <typename T, class P>
  typename std::enable_if<is_smart_pointer<T>::value && is_instantiation<std::map, typename T::element_type>::value>::type
    LuaCppContainer::push(lua_State *state, LuaCppRuntime &runtime, T &value) {
    LuaCppMap<P>::push(state, runtime, value);
  }
  
  template <typename T, class P>
  typename std::enable_if<LuaCppTuple::is_tuple<T>()>::type
    LuaCppContainer::push(lua_State *state, LuaCppRuntime &runtime, T &value) {
    LuaCppTuple::push<T, P>(state, runtime, value);
  }

  template <typename T, class P>
  typename std::enable_if<is_instantiation<std::optional, T>::value>::type
    LuaCppContainer::push(lua_State *state, LuaCppRuntime &runtime, T &value) {
    if (value.has_value()) {
      P::push(state, runtime, value.value());
    } else {
      Internal::LuaStack stack(state);
      stack.push();
    }
  }
}

#endif