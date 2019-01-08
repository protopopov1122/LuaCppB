#ifndef LUACPPB_OBJECT_NATIVE_H_
#define LUACPPB_OBJECT_NATIVE_H_

#include "luacppb/Base.h"
#include "luacppb/Core/Runtime.h"
#include "luacppb/Core/Stack.h"
#include <type_traits>

namespace LuaCppB::Internal {

  class LuaNativeObject {
   public:
    template <typename T>
    static typename std::enable_if<std::is_pointer<T>::value>::type push(lua_State *, LuaCppRuntime &, T);

    template <typename T>
    static typename std::enable_if<!std::is_pointer<T>::value && !is_smart_pointer<T>::value>::type
      push(lua_State *, LuaCppRuntime &, T &);

    template <typename T>
    static typename std::enable_if<is_instantiation<std::unique_ptr, T>::value>::type
      push(lua_State *, LuaCppRuntime &, T &);

    template <typename T>
    static typename std::enable_if<is_instantiation<std::shared_ptr, T>::value>::type
      push(lua_State *, LuaCppRuntime &, T &);
  };
}

#include "luacppb/Object/Impl/Native.h"

#endif