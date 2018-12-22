#ifndef LUACPPB_VALUE_NATIVE_H_
#define LUACPPB_VALUE_NATIVE_H_

#include "luacppb/Base.h"
#include "luacppb/Value/Value.h"
#include "luacppb/Core/Runtime.h"
#include "luacppb/Object/Native.h"
#include "luacppb/Container/Container.h"
#include <type_traits>
#include <functional>

namespace LuaCppB {

  template <typename T>
  struct LuaNativeValueSpecialCase {
    static constexpr bool value = is_smart_pointer<T>::value ||
                                  is_instantiation<std::reference_wrapper, T>::value ||
                                  is_instantiation<std::vector, T>::value;
  };

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
    static typename std::enable_if<!LuaValue::is_constructible<T>() && !LuaNativeValueSpecialCase<T>::value>::type
      push(lua_State *state, LuaCppRuntime &runtime, T &value) {
      LuaNativeObject::push<T>(state, runtime, value);
    }
    
    template <typename T>
    static typename std::enable_if<is_smart_pointer<T>::value && !is_container<typename T::element_type>::value>::type
      push(lua_State *state, LuaCppRuntime &runtime, T &value) {
      LuaNativeObject::push<T>(state, runtime, value);
    }

    template <typename T>
    static typename std::enable_if<is_instantiation<std::reference_wrapper, T>::value>::type
      push(lua_State *state, LuaCppRuntime &runtime, T &value) {
      LuaNativeValue::push<typename T::type>(state, runtime, value.get());
    }

    template <typename T>
    static typename std::enable_if<is_instantiation<std::vector, T>::value>::type
      push(lua_State *state, LuaCppRuntime &runtime, T &value) {
      using E = typename T::value_type;
      void (*pusher)(lua_State *, LuaCppRuntime &, E &) = &LuaNativeValue::push<E>;
      LuaCppContainer::push(state, runtime, value, pusher);
    }

    template <typename T>
    static typename std::enable_if<is_smart_pointer<T>::value && is_instantiation<std::vector, typename T::element_type>::value>::type
      push(lua_State *state, LuaCppRuntime &runtime, T &value) {
      using E = typename T::element_type::value_type;
      void (*pusher)(lua_State *, LuaCppRuntime &, E &) = &LuaNativeValue::push<E>;
      LuaCppContainer::push(state, runtime, value, pusher);
    }
  };
}

#endif