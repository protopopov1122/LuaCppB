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

#ifndef LUACPPB_META_H_
#define LUACPPB_META_H_

#include "luacppb/Base.h"
#include <type_traits>
#include <memory>

namespace LuaCppB::Internal {
	template<typename T>
	struct always_false : std::false_type {};

	template<template<typename...> class, typename...>
	struct is_instantiation : public std::false_type {};

	template<template<typename...> class U, typename... T>
	struct is_instantiation<U, U<T...>> : public std::true_type {};

	template <typename T>
	struct is_smart_pointer {
		static constexpr bool value = is_instantiation<std::unique_ptr, T>::value || is_instantiation<std::shared_ptr, T>::value;
	};

	template <typename T, typename E = void>
	struct is_callable : std::is_function<T> {};

	template <typename T>
	struct is_callable<T, typename std::enable_if<std::is_pointer<T>::value>::type> : std::is_function<typename std::remove_pointer<T>::type> {};

	template <typename T>
	struct is_callable<T, typename std::enable_if<std::is_same<decltype(void(&T::operator())), void>::value>::type> : std::true_type {};
}

#endif
