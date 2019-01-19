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

#ifndef LUACPPB_REFERENCE_HANDLE_H_
#define LUACPPB_REFERENCE_HANDLE_H_

#include "luacppb/Meta.h"
#include "luacppb/Reference/Base.h"
#include "luacppb/Value/Native.h"
#include "luacppb/Invoke/Native.h"
#include "luacppb/Invoke/Invoke.h"
#include <memory>
#include <utility>
#include <type_traits>

namespace LuaCppB {

	class TableIterator;

	class LuaReferenceHandle {
	 public:
	 	LuaReferenceHandle();
		LuaReferenceHandle(lua_State *, std::unique_ptr<Internal::LuaReference>);
		LuaReferenceHandle(const LuaReferenceHandle &);
		LuaReferenceHandle(LuaReferenceHandle &&);

		Internal::LuaReference &getReference() const;
		LuaCppRuntime &getRuntime() const;
		bool isValid() const;
		bool exists();
		LuaType getType();
		LuaReferenceHandle operator[](const std::string &);
		LuaReferenceHandle operator[](const char *);
		LuaReferenceHandle operator[](lua_Integer);
		LuaReferenceHandle &operator=(const LuaReferenceHandle &);
		LuaValue operator*() const;
		TableIterator begin();
		const TableIterator &end() const;

		LuaReferenceHandle getMetatable();
		void setMetatable(LuaData &);
		void setMetatable(LuaData &&);

		bool operator==(const LuaReferenceHandle &) const;
		bool operator!=(const LuaReferenceHandle &) const;

		template <typename T>
		typename std::enable_if<!std::is_same<T, LuaReferenceHandle>::value, LuaReferenceHandle>::type &operator=(T &);

		template <typename T>
		typename std::enable_if<!std::is_same<T, LuaReferenceHandle>::value, LuaReferenceHandle>::type &operator=(T &&);

		template <typename T, typename Type = typename std::enable_if<!std::is_class<T>::value || Internal::LuaReferenceGetSpecialCase<T>::value, T>::type>
		operator T ();

		template <typename T, typename Type = typename std::enable_if<std::is_class<T>::value && !Internal::LuaReferenceGetSpecialCase<T>::value, T>::type>
		operator T& ();

		template <typename T>
		typename std::enable_if<!Internal::is_instantiation<std::function, T>::value, T>::type get() const;

		template <typename ... A>
		Internal::LuaFunctionCallResult operator()(A &&...) const;
	 private:
	 	lua_State *state;
		std::unique_ptr<Internal::LuaReference> ref;
	};
}

#include "luacppb/Reference/Impl/Handle.h"

#endif
