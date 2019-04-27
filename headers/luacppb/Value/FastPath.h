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

#ifndef LUACPPB_VALUE_FAST_PATH_H_
#define LUACPPB_VALUE_FAST_PATH_H_

#include "luacppb/Base.h"
#include <type_traits>

namespace LuaCppB::Internal {

  class LuaValueFastPath {
   public:
    template <typename T>
    static typename std::enable_if<std::is_integral<T>::value && !std::is_same<T, bool>::value, T>::type peek(lua_State *, int = -1);

    template <typename T>
    static typename std::enable_if<std::is_floating_point<T>::value, T>::type peek(lua_State *, int = -1);

    template <typename T>
    static typename std::enable_if<std::is_same<T, bool>::value, T>::type peek(lua_State *, int = -1);

    template <typename T>
    static typename std::enable_if<std::is_integral<T>::value && !std::is_same<T, bool>::value>::type push(lua_State *, T);

    template <typename T>
    static typename std::enable_if<std::is_floating_point<T>::value>::type push(lua_State *, T);

    template <typename T>
    static typename std::enable_if<std::is_same<T, bool>::value>::type push(lua_State *, T);

    template <typename T>
    static constexpr bool isSupported() {
      return (std::is_integral<T>::value && !std::is_same<T, bool>::value) ||
        std::is_floating_point<T>::value ||
        std::is_same<T, bool>::value;
    }
  };
}

#include "luacppb/Value/Impl/FastPath.h"

#endif