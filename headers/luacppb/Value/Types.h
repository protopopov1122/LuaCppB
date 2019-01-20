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

#ifndef LUACPPB_VALUE_TYPES_H_
#define LUACPPB_VALUE_TYPES_H_

#include "luacppb/Value/Base.h"
#include "luacppb/Reference/Registry.h"
#include "luacppb/Meta.h"
#include "luacppb/Core/Runtime.h"
#include "luacppb/Core/Status.h"
#include <string>

namespace LuaCppB {

	class LuaNil : public LuaValueBase {
	 public:
		void push(lua_State *) const override;
		static LuaNil get(lua_State *, int = -1);
	};

	class LuaInteger : public LuaValueBase {
	 public:
	 	LuaInteger();
		LuaInteger(lua_Integer);
		operator lua_Integer() const;
		void push(lua_State *) const override;
		static LuaInteger get(lua_State *, int = -1);
	 private:
		lua_Integer integer;
	};

	class LuaNumber : public LuaValueBase {
	 public:
	 	LuaNumber();
	 	LuaNumber(lua_Number);
		operator lua_Number() const;
		void push(lua_State *) const override;
		static LuaNumber get(lua_State *, int = -1);
	 private:
		lua_Number number;
	};

	class LuaBoolean : public LuaValueBase {
	 public:
		LuaBoolean(bool);
		operator bool() const;
		void push(lua_State *) const override;
		static LuaBoolean get(lua_State *, int = -1);
	 private:
		bool boolean;
	};

	class LuaString : public LuaValueBase {
	 public:
		LuaString(const std::string &);
		LuaString(const char *);
	 	operator const std::string &() const;
		void push(lua_State *) const override;
		static LuaString get(lua_State *, int = -1);
	 private:
		std::string string;
	};

	typedef int (*LuaCFunction)(lua_State *);

	class LuaReferenceHandle;

	class LuaReferencedValue : public LuaValueBase {
	 public:
	 	LuaReferencedValue();
		LuaReferencedValue(lua_State *, int = -1);
		LuaReferencedValue(const LuaReferencedValue &);
		LuaReferencedValue(Internal::LuaSharedRegistryHandle &);
		LuaReferencedValue &operator=(const LuaReferencedValue &);
		
		void push(lua_State *) const override;
		bool hasValue() const;
		LuaReferenceHandle ref(LuaCppRuntime &);

	protected:
		template <typename T>
		T toPointer() const;

	 	Internal::LuaSharedRegistryHandle handle;
	};

	class LuaTable : public LuaReferencedValue {
	 public:
	 	using LuaReferencedValue::LuaReferencedValue;
		static LuaTable get(lua_State *, int = -1);
		static LuaTable create(lua_State *);
	};

	class LuaUserData : public LuaReferencedValue {
	 public:
	 	using LuaReferencedValue::LuaReferencedValue;
		static LuaUserData get(lua_State *, int = -1);
		static LuaUserData create(lua_State *, std::size_t);

		template <typename T>
		static std::string getCustomName(uint64_t);

		template <typename T>
		T *getCustomData() const;

		template <typename T>
		T toPointer() const;
	};

	class LuaThread : public LuaReferencedValue {
	 public:
		using LuaReferencedValue::LuaReferencedValue;
		lua_State *toState() const;
		LuaStatusCode status() const;
		static LuaThread get(lua_State *, int = -1);
		static LuaThread create(lua_State *);
	};

	class LuaFunction : public LuaReferencedValue {
	 public:
		using LuaReferencedValue::LuaReferencedValue;
		bool isCFunction() const;
		LuaCFunction toCFunction() const;
		static LuaFunction get(lua_State *, int = -1);
		static LuaFunction create(lua_State *, LuaCFunction, int = 0);
	};
}

#include "luacppb/Value/Impl/Types.h"

#endif 
