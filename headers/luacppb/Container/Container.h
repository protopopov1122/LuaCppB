#ifndef LUACPPB_CONTAINER_CONTAINER_H_
#define LUACPPB_CONTAINER_CONTAINER_H_

#include "luacppb/Container/Vector.h"
#include "luacppb/Container/Map.h"
#include "luacppb/Container/Tuple.h"
#include "luacppb/Core/Stack.h"
#include "luacppb/Meta.h"
#include <type_traits>
#include <optional>

namespace LuaCppB::Internal {

  class LuaCppContainer {
   public:
    // Vectors
    template <typename T, class P>
    static typename std::enable_if<is_instantiation<std::vector, T>::value>::type
      push(lua_State *state, LuaCppRuntime &runtime, T &value) {
      LuaCppVector<P>::push(state, runtime, value);
    }

    template <typename T, class P>
    static typename std::enable_if<std::is_const<T>::value && is_instantiation<std::vector, typename std::remove_const<T>::type>::value>::type
      push(lua_State *state, LuaCppRuntime &runtime, T &value) {
      LuaCppVector<P>::push(state, runtime, value);
    }

    template <typename T, class P>
    static typename std::enable_if<is_smart_pointer<T>::value && is_instantiation<std::vector, typename T::element_type>::value>::type
      push(lua_State *state, LuaCppRuntime &runtime, T &value) {
      LuaCppVector<P>::push(state, runtime, value);
    }
    // Map
    template <typename T, class P>
    static typename std::enable_if<is_instantiation<std::map, T>::value>::type
      push(lua_State *state, LuaCppRuntime &runtime, T &value) {
      LuaCppMap<P>::push(state, runtime, value);
    }

    template <typename T, class P>
    static typename std::enable_if<std::is_const<T>::value && is_instantiation<std::map, typename std::remove_const<T>::type>::value>::type
      push(lua_State *state, LuaCppRuntime &runtime, T &value) {
      LuaCppMap<P>::push(state, runtime, value);
    }

    template <typename T, class P>
    static typename std::enable_if<is_smart_pointer<T>::value && is_instantiation<std::map, typename T::element_type>::value>::type
      push(lua_State *state, LuaCppRuntime &runtime, T &value) {
      LuaCppMap<P>::push(state, runtime, value);
    }
    // Pair & tuple
    template <typename T, class P>
    static typename std::enable_if<LuaCppTuple::is_tuple<T>()>::type
      push(lua_State *state, LuaCppRuntime &runtime, T &value) {
      LuaCppTuple::push<T, P>(state, runtime, value);
    }
    // Optional
    template <typename T, class P>
    static typename std::enable_if<is_instantiation<std::optional, T>::value>::type
      push(lua_State *state, LuaCppRuntime &runtime, T &value) {
      if (value.has_value()) {
        P::push(state, runtime, value.value());
      } else {
        Internal::LuaStack stack(state);
        stack.push();
      }
    }

    template <typename T>
    static constexpr bool is_container() {
      return is_instantiation<std::vector, T>::value ||
             is_instantiation<std::map, T>::value ||
             is_instantiation<std::optional, T>::value ||
             LuaCppTuple::is_tuple<T>() ||
             (std::is_const<T>::value && LuaCppContainer::is_container<typename std::remove_const<T>::type>());
    }

  };
}

#endif