#ifndef LUACPPB_VALUE_TYPES_H_
#define LUACPPB_VALUE_TYPES_H_

#include "luacppb/Value/Base.h"
#include "luacppb/Reference/Registry.h"
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
		void push(lua_State *) const override;
		static LuaInteger get(lua_State *, int = -1);

		operator lua_Integer() const {
			return this->integer;
		}
	 private:
		lua_Integer integer;
	};

	class LuaNumber : public LuaValueBase {
	 public:
	 	LuaNumber();
	 	LuaNumber(lua_Number);
		operator lua_Number() const {
			return this->number;
		}
		void push(lua_State *) const override;
		static LuaNumber get(lua_State *, int = -1);
	 private:
		lua_Number number;
	};

	class LuaBoolean : public LuaValueBase {
	 public:
		LuaBoolean(bool);
		operator bool() const {
			return this->boolean;
		}
		void push(lua_State *) const override;
		static LuaBoolean get(lua_State *, int = -1);
	 private:
		bool boolean;
	};

	class LuaString : public LuaValueBase {
	 public:
		LuaString(const std::string &);
		LuaString(const char *);
	 	operator const std::string &() const {
			return this->string;
		}
		void push(lua_State *) const override;
		static LuaString get(lua_State *, int = -1);
	 private:
		std::string string;
	};

	typedef int (*LuaCFunction_ptr)(lua_State *);

	class LuaCFunction : public LuaValueBase {
	 public:
		LuaCFunction(LuaCFunction_ptr);
		operator LuaCFunction_ptr() const {
			return this->function;
		}
		void push(lua_State *) const override;
		static LuaCFunction get(lua_State *, int = -1);
	 private:
		LuaCFunction_ptr function;
	};

	template<typename T>
	struct always_false : std::false_type {};

	class LuaReferenceHandle;

	class LuaTableBase : public LuaValueBase {
	 public:
	 	LuaTableBase();
		LuaTableBase(lua_State *, int = -1);
		LuaTableBase(const LuaTableBase &);
		
		void push(lua_State *) const override;
		static LuaTableBase get(lua_State *, int = -1);
		static LuaTableBase create(lua_State *);

		template <typename T>
		operator T() {
			return this->convert<T>();
		}
	 private:
		template <typename T>
		typename std::enable_if<!std::is_same<T, LuaReferenceHandle>::value, T>::type convert() {
			static_assert(always_false<T>::value , "");
		}

		template <typename T>
		typename std::enable_if<std::is_same<T, LuaReferenceHandle>::value, T>::type convert();

	 	LuaRegistryHandle handle;
	};
}

#endif 