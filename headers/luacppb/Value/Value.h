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

#ifndef LUACPPB_VALUE_H_
#define LUACPPB_VALUE_H_

#include "luacppb/Base.h"
#include "luacppb/Core/Error.h"
#include "luacppb/Value/Types.h"
#include "luacppb/Object/Wrapper.h"
#include <memory>
#include <variant>
#include <type_traits>
#include <optional>
#include <cassert>

namespace LuaCppB {

	namespace Internal {
		template <typename T>
		struct LuaValueGetSpecialCase {
			using T2 = typename std::decay<T>::type;
			static constexpr bool value = std::is_same<T, std::string>::value ||
																		std::is_same<T2, LuaTable>::value ||
																		std::is_same<T2, LuaUserData>::value ||
																		std::is_same<T2, LuaThread>::value ||
																		std::is_same<T2, LuaFunction>::value ||
																		std::is_same<T2, LuaEmpty>::value;
		};

		template <typename T, typename E = typename std::enable_if<std::is_reference<T>::value>::type>
		struct LuaValueGetSpecialRef {
			using V = typename std::remove_reference<T>::type;
			static constexpr bool value = std::is_const<V>::value && std::is_same<typename std::decay<T>::type, std::string>::value;
		};
	}

	class LuaValue : public LuaData {
	 public:
		LuaValue();
		LuaValue(LuaInteger);
		LuaValue(LuaNumber);
		LuaValue(LuaBoolean);
		LuaValue(const LuaString &);
		LuaValue(LuaFunction);
		LuaValue(LuaCFunction);
		LuaValue(LuaTable);
		LuaValue(void *);
		LuaValue(LuaUserData);
		LuaValue(LuaThread);
		
		LuaValue(LuaValue &&);
		LuaValue(const LuaValue &);
		LuaValue &operator=(const LuaValue &);

		LuaType getType() const noexcept;
		void push(lua_State *state) const override;
		static std::optional<LuaValue> peek(lua_State *, lua_Integer = -1);
		static std::optional<LuaValue>	fromRegistry(Internal::LuaSharedRegistryHandle &);

		static LuaValue Nil;

		template <typename T>
		typename std::enable_if<std::is_integral<T>::value && !std::is_same<T, bool>::value, T>::type get(T = 0) const;

		template <typename T>
		typename std::enable_if<std::is_floating_point<T>::value, T>::type get(T = 0.0f) const;

		template <typename T>
		typename std::enable_if<std::is_same<T, bool>::value, T>::type get(T = false) const;

		template <typename T>
		typename std::enable_if<std::is_same<T, std::string>::value, T>::type get(const T & = "") const;

		template <typename T>
		typename std::enable_if<std::is_reference<T>::value && std::is_same<typename std::decay<T>::type, std::string>::value, T>::type get(const T & = "") const;

		template <typename T>
		typename std::enable_if<std::is_same<T, LuaCFunction>::value, T>::type get(T = nullptr) const;

		template <typename T>
		typename std::enable_if<std::is_same<T, LuaTable>::value, T>::type get() const;

		template <typename T>
		typename std::enable_if<std::is_same<T, LuaUserData>::value, T>::type get() const;

		template <typename T>
		typename std::enable_if<std::is_pointer<T>::value && std::is_class<typename std::remove_pointer<T>::type>::value, T>::type get(T defaultValue = nullptr) const;

		template <typename T>
		typename std::enable_if<std::is_reference<T>::value && std::is_class<typename std::decay<T>::type>::value && !Internal::LuaValueGetSpecialRef<T>::value, T>::type get() const;

		template <typename T>
		typename std::enable_if<std::is_class<T>::value && !Internal::LuaValueGetSpecialCase<T>::value, T>::type get() const;

		template <typename T>
		typename std::enable_if<std::is_same<T, LuaThread>::value, T>::type get() const;

		template <typename T>
		typename std::enable_if<std::is_same<T, LuaFunction>::value, T>::type get() const;

		template <typename T>
		typename std::enable_if<std::is_enum<T>::value, T>::type get() const;

		template <typename T>
		typename std::enable_if<std::is_same<T, LuaEmpty>::value, T>::type get() const;

		template <typename T, typename Type = typename std::enable_if<!std::is_class<T>::value || Internal::LuaValueGetSpecialCase<T>::value, T>::type>
		operator T ();

		template <typename T, typename Type = typename std::enable_if<std::is_class<T>::value && !Internal::LuaValueGetSpecialCase<T>::value, T>::type>
		operator T& ();

		template <typename T>
		static typename std::enable_if<std::is_integral<T>::value && !std::is_same<T, bool>::value, LuaValue>::type create(T) noexcept;

		template <typename T>
		static typename std::enable_if<std::is_floating_point<T>::value, LuaValue>::type create(T) noexcept;

		template <typename T>
		static typename std::enable_if<std::is_same<T, bool>::value, LuaValue>::type create(T) noexcept;

		template <typename T>
		static typename std::enable_if<std::is_same<T, LuaCFunction>::value, LuaValue>::type create(T) noexcept;

		template <typename T>
		static typename std::enable_if<std::is_same<typename std::decay<T>::type, const char *>::value, LuaValue>::type
			create(T) noexcept;

		template <typename T>
		static typename std::enable_if<std::is_same<typename std::decay<T>::type, std::string>::value, LuaValue>::type
			create(T) noexcept;

		template <typename T>
		static constexpr bool is_constructible() noexcept {
			return std::is_integral<T>::value ||
				std::is_floating_point<T>::value ||
				std::is_same<T, bool>::value ||
				std::is_same<T, LuaCFunction>::value ||
				std::is_same<typename std::decay<T>::type, const char *>::value ||
				std::is_same<typename std::decay<T>::type, std::string>::value;
		}
	 private:
		LuaType type;
	 	std::variant<LuaInteger, LuaNumber, LuaBoolean, LuaString, LuaCFunction, LuaTable, void *, LuaUserData, LuaThread, LuaFunction> value;
	};

	namespace Internal {

		template <typename T, typename E = void>
		struct LuaValueWrapper {
			static constexpr bool Conversible = false;
		};

		template <typename T>
		struct LuaValueWrapper<T, typename std::enable_if<LuaValue::is_constructible<T>()>::type> {
			static LuaValue wrap(T value) {
			  return LuaValue::create(value);
			}

			static constexpr bool Conversible = true;
		};

		template <typename T>
		struct LuaValueWrapper<T, typename std::enable_if<std::is_convertible<T, LuaValue>::value>::type> {
			static LuaValue wrap(T value) {
			  return LuaValue(value);
			}

			static constexpr bool Conversible = true;
		};
	}
}

#endif
