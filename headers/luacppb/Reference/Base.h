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

#ifndef LUACPPB_REFERENCE_BASE_H_
#define LUACPPB_REFERENCE_BASE_H_

#include "luacppb/Base.h"
#include "luacppb/Value/Value.h"
#include "luacppb/Value/Native.h"
#include "luacppb/Invoke/Coro.h"
#include "luacppb/Core/Runtime.h"
#include "luacppb/Value/FastPath.h"
#include <optional>
#include <type_traits>
#include <functional>

namespace LuaCppB::Internal {

#ifdef LUACPPB_COROUTINE_SUPPORT
	template <typename T>
	struct LuaReferenceIsCoroutine {
		using T2 = typename std::remove_cv<typename std::remove_reference<T>::type>::type;
		static constexpr bool value = std::is_same<T2, LuaCoroutine>::value;
	};

	template <typename T>
	struct LuaReferenceGetSpecialCase {
		static constexpr bool value = LuaValueGetSpecialCase<T>::value ||
			                          LuaReferenceIsCoroutine<T>::value;
	};
#else
	template <typename T>
	using LuaReferenceGetSpecialCase = LuaValueGetSpecialCase<T>;
#endif

	class LuaReference {
	 public:
	 	LuaReference(LuaCppRuntime &);
		virtual ~LuaReference() = default;
		LuaCppRuntime &getRuntime();

		virtual bool putOnTop(std::function<void (lua_State *)>) = 0;
		virtual bool setValue(std::function<void (lua_State *)>) = 0;
		virtual lua_State *putOnTop() = 0;
		virtual std::unique_ptr<LuaReference> clone(LuaCppRuntime &) = 0;

		template <typename T>
		typename std::enable_if<std::is_same<T, LuaValue>::value, T>::type get();
		
		template <typename T>
		typename std::enable_if<Internal::LuaValueFastPath::isSupported<T>(), T>::type get();
#ifdef LUACPPB_COROUTINE_SUPPORT
		template <typename T>
		typename std::enable_if<std::is_convertible<LuaValue, T>::value && !std::is_same<T, LuaValue>::value && !Internal::LuaValueFastPath::isSupported<T>() && !LuaReferenceIsCoroutine<T>::value, T>::type
			get();

		template <typename T>
		typename std::enable_if<LuaReferenceIsCoroutine<T>::value, T>::type
			get();
#else
		template <typename T>
		typename std::enable_if<std::is_convertible<LuaValue, T>::value && !std::is_same<T, LuaValue>::value && !Internal::LuaValueFastPath::isSupported<T>(), T>::type
			get();
#endif

		template <typename T>
		typename std::enable_if<std::is_base_of<LuaData, T>::value>::type set(T &);

		template <typename T>
		typename std::enable_if<!std::is_base_of<LuaData, T>::value>::type set(T &);
	 protected:
		virtual LuaValue toValue();

		LuaCppRuntime &runtime;
	};
}

#include "luacppb/Reference/Impl/Base.h"

#endif
