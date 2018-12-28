#ifndef LUACPPB_VALUE_NATIVE_H_
#define LUACPPB_VALUE_NATIVE_H_

#include "luacppb/Base.h"
#include "luacppb/Value/Value.h"
#include "luacppb/Invoke/Native.h"
#include "luacppb/Core/Runtime.h"
#include "luacppb/Object/Native.h"
#include "luacppb/Container/Container.h"
#include <type_traits>
#include <functional>

namespace LuaCppB::Internal {


  template <typename T>
  struct LuaNativeValueSpecialCase {
    static constexpr bool value = std::is_base_of<LuaData, T>::value ||
                                  LuaValue::is_constructible<T>() ||
                                  is_smart_pointer<T>::value ||
                                  is_instantiation<std::reference_wrapper, T>::value ||
                                  is_callable<T>::value ||
                                  std::is_enum<T>::value ||
                                  Internal::LuaCppContainer::is_container<T>();
  };

  class LuaNativeValue {
   public:
    // LuaData values
    template <typename T>
    static typename std::enable_if<std::is_base_of<LuaData, T>::value>::type
      push(lua_State *state, LuaCppRuntime &runtime, T &value) {
      value.push(state);
    }
    // Trivial values
    template <typename T>
    static typename std::enable_if<LuaValue::is_constructible<T>()>::type
      push(lua_State *state, LuaCppRuntime &runtime, T &value) {
      LuaValue::create<T>(value).push(state);
    }

    template <typename T>
    static typename std::enable_if<LuaValue::is_constructible<T>()>::type
      push(lua_State *state, LuaCppRuntime &runtime, T &&value) {
      LuaValue::create<T>(value).push(state);
    }
    // Enum
    template <typename T>
    static typename std::enable_if<std::is_enum<T>::value>::type
      push(lua_State *state, LuaCppRuntime &runtime, T &value) {
      using EnumType = typename std::underlying_type<T>::type;
      LuaNativeValue::push(state, runtime, static_cast<EnumType>(value));
    }
    // Arbitrary objects
    template <typename T>
    static typename std::enable_if<!LuaNativeValueSpecialCase<T>::value>::type
      push(lua_State *state, LuaCppRuntime &runtime, T &value) {
      Internal::LuaNativeObject::push<T>(state, runtime, value);
    }
    
    template <typename T>
    static typename std::enable_if<is_smart_pointer<T>::value && !Internal::LuaCppContainer::is_container<typename T::element_type>()>::type
      push(lua_State *state, LuaCppRuntime &runtime, T &value) {
      Internal::LuaNativeObject::push<T>(state, runtime, value);
    }
    // Reference wrapped values
    template <typename T>
    static typename std::enable_if<is_instantiation<std::reference_wrapper, T>::value>::type
      push(lua_State *state, LuaCppRuntime &runtime, T &value) {
      LuaNativeValue::push<typename T::type>(state, runtime, value.get());
    }
    // Containers
    template <typename T>
    static typename std::enable_if<Internal::LuaCppContainer::is_container<T>()>::type
      push(lua_State *state, LuaCppRuntime &runtime, T &value) {
      Internal::LuaCppContainer::push<T, Internal::LuaNativeValue>(state, runtime, value);
    }

    template <typename T>
    static typename std::enable_if<is_smart_pointer<T>::value && Internal::LuaCppContainer::is_container<typename T::element_type>()>::type
      push(lua_State *state, LuaCppRuntime &runtime, T &value) {
      Internal::LuaCppContainer::push<T, Internal::LuaNativeValue>(state, runtime, value);
    }
    // Callables
    template <typename T>
    static typename std::enable_if<is_callable<T>::value>::type
      push(lua_State *state, LuaCppRuntime &runtime, T value) {
      NativeInvocable<Internal::LuaNativeValue>::create(value, runtime).push(state);
    }
  };
}

#endif