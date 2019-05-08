/*
  SPDX short identifier: MIT

  Copyright 2018-2019 Jevgēnijs Protopopovs

  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"),
  to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
  and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
  IN THE SOFTWARE.
*/

#ifndef LUACPPB_VALUE_NATIVE_H_
#define LUACPPB_VALUE_NATIVE_H_

#include "luacppb/Base.h"
#include "luacppb/Value/Value.h"
#include "luacppb/Value/FastPath.h"
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
                                  Internal::LuaCppContainer::is_container<T>() ||
                                  std::is_same<T, LuaFunctionCallResult>::value;
  };
  
  template <typename T>
  struct LuaNativeValueObjectSmartPointer_Impl {
	static constexpr bool value() {
		if constexpr (is_smart_pointer<T>::value) {
			return !Internal::LuaCppContainer::is_container<typename T::element_type>();
		} else {
			return false;
		}
	}
  };
  
  template <typename T>
  struct LuaNativeValueObjectSmartPointer {
	static constexpr bool value = LuaNativeValueObjectSmartPointer_Impl<T>::value();
  };
  
  template <typename T>
  struct LuaNativeValueContainer {
	static constexpr bool value = Internal::LuaCppContainer::is_container<T>();
  };
  
  template <typename T>
  struct LuaNativeValueContainerSmartPointer_Impl {
	static constexpr bool value() {
		if constexpr (is_smart_pointer<T>::value) {
			return Internal::LuaCppContainer::is_container<typename T::element_type>();
		} else {
			return false;
		}
	}
  };
  
  template <typename T>
  struct LuaNativeValueContainerSmartPointer {
	static constexpr bool value = LuaNativeValueContainerSmartPointer_Impl<T>::value();
  };

  class LuaNativeValue {
   public:
    template <typename T>
    static typename std::enable_if<std::is_base_of<LuaData, T>::value>::type
      push(lua_State *, LuaCppRuntime &, T &);
    
    template <typename T>
    static typename std::enable_if<LuaValueFastPath::isSupported<T>()>::type
      push(lua_State *, LuaCppRuntime &, T);

    template <typename T>
    static typename std::enable_if<LuaValue::is_constructible<T>() && !LuaValueFastPath::isSupported<T>()>::type
      push(lua_State *, LuaCppRuntime &, T &);

    template <typename T>
    static typename std::enable_if<LuaValue::is_constructible<T>() && !LuaValueFastPath::isSupported<T>()>::type
      push(lua_State *, LuaCppRuntime &, T &&);
    
    template <typename T>
    static typename std::enable_if<std::is_enum<T>::value>::type
      push(lua_State *, LuaCppRuntime &, T &);
    
    template <typename T>
    static typename std::enable_if<!LuaNativeValueSpecialCase<T>::value>::type
      push(lua_State *, LuaCppRuntime &, T &);
    
    template <typename T>
    static typename std::enable_if<LuaNativeValueObjectSmartPointer<T>::value>::type
      push(lua_State *, LuaCppRuntime &, T &);
    
    template <typename T>
    static typename std::enable_if<is_instantiation<std::reference_wrapper, T>::value>::type
      push(lua_State *, LuaCppRuntime &, T &);
    
    template <typename T>
    static typename std::enable_if<LuaNativeValueContainer<T>::value>::type
      push(lua_State *, LuaCppRuntime &, T &);

    template <typename T>
    static typename std::enable_if<LuaNativeValueContainerSmartPointer<T>::value>::type
      push(lua_State *, LuaCppRuntime &, T &);
    
    template <typename T>
    static typename std::enable_if<is_callable<T>::value && !is_instantiation<std::reference_wrapper, T>::value>::type
      push(lua_State *, LuaCppRuntime &, T &);
    
    template <typename T>
    static typename std::enable_if<std::is_same<T, LuaFunctionCallResult>::value>::type
      push(lua_State *, LuaCppRuntime &, T &);
  };
}

#endif
