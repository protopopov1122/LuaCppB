#ifndef LUACPPB_VALUE_H_
#define LUACPPB_VALUE_H_

#include "luacppb/Base.h"
#include <variant>
#include <type_traits>
#include <iostream>
#include <optional>

namespace LuaCppB {

	enum class LuaType {
		None = LUA_TNONE,
		Nil = LUA_TNIL,
		Number = LUA_TNUMBER,
		Boolean = LUA_TBOOLEAN,
		String = LUA_TSTRING,
		Table = LUA_TTABLE,
		Function = LUA_TFUNCTION,
		UserData = LUA_TUSERDATA,
		Thread = LUA_TTHREAD,
		LightUserData = LUA_TLIGHTUSERDATA
	};

	class LuaData {
	 public:
		virtual ~LuaData() = default;
		virtual void push(lua_State *state) const = 0;
	};

	class LuaNil : public LuaData {
	 public:
		void push(lua_State *state) const override {
			lua_pushnil(state);
		}

		static LuaNil get(lua_State *state, int index = -1) {
			return LuaNil();
		}
	};

	class LuaInteger : public LuaData {
	 public:
	 	LuaInteger() : integer(0) {}
		LuaInteger(lua_Integer i) : integer(i) {}
		operator lua_Integer() const {
			return this->integer;
		}
		void push(lua_State *state) const override {
			lua_pushinteger(state, this->integer);
		}
		static LuaInteger get(lua_State *state, int index = -1) {
			return LuaInteger(lua_tointeger(state, index));
		}
	 private:
		lua_Integer integer;
	};

	class LuaNumber : public LuaData {
	 public:
	 	LuaNumber() : number(0) {}
	 	LuaNumber(lua_Number n) : number(n) {}
		operator lua_Number() const {
			return this->number;
		}
		void push(lua_State *state) const override {
			lua_pushnumber(state, this->number);
		}
		static LuaNumber get(lua_State *state, int index = -1) {
			return LuaNumber(lua_tonumber(state, index));
		}
	 private:
		lua_Number number;
	};

	class LuaBoolean : public LuaData {
	 public:
		LuaBoolean(bool b) : boolean(b) {}
		operator bool() const {
			return this->boolean;
		}
		void push(lua_State *state) const override {
			lua_pushboolean(state, static_cast<int>(this->boolean));
		}
		static LuaBoolean get(lua_State *state, int index = -1) {
			return LuaBoolean(static_cast<bool>(lua_toboolean(state, index)));
		}
	 private:
		bool boolean;
	};

	class LuaString : public LuaData {
	 public:
		LuaString(const std::string &str) : string(str) {}
		LuaString(const char *str) : string(str) {}
	 	operator const std::string &() const {
			return this->string;
		}
		void push(lua_State *state) const override {
			lua_pushstring(state, this->string.c_str());
		}
		static LuaString get(lua_State *state, int index = -1) {
			return LuaString(lua_tostring(state, index));
		}
	 private:
		std::string string;
	};

	typedef int (*LuaCFunction_ptr)(lua_State *);

	class LuaCFunction : public LuaData {
	 public:
		LuaCFunction(LuaCFunction_ptr fn) : function(fn) {}
		operator LuaCFunction_ptr() const {
			return this->function;
		}
		void push(lua_State *state) const override {
			lua_pushcfunction(state, this->function);
		}
		static LuaCFunction get(lua_State *state, int index = -1) {
			return LuaCFunction(lua_tocfunction(state, index));
		}
	 private:
		LuaCFunction_ptr function;
	};

	class LuaValue : public LuaData {
	 public:
		LuaValue() : type(LuaType::Nil) {}
		LuaValue(LuaInteger i) : type(LuaType::Number), value(i) {}
		LuaValue(LuaNumber n) : type(LuaType::Number), value(n) {}
		LuaValue(LuaBoolean b) : type(LuaType::Boolean), value(b) {}
		LuaValue(const LuaString &s) : type(LuaType::String), value(s) {}
		LuaValue(LuaCFunction f) : type(LuaType::Function), value(f) {}

		LuaType getType() const;
		void push(lua_State *state) const override;
		static std::optional<LuaValue> peek(lua_State *, lua_Integer = -1);

		template <typename T>
		typename std::enable_if<std::is_integral<T>::value && !std::is_same<T, bool>::value, T>::type get(T defaultValue = 0) const {
			if (this->type == LuaType::Number) {
				if (this->value.index() == 0) {
					return static_cast<T>(std::get<LuaInteger>(this->value));
				} else {
					return static_cast<T>(std::get<LuaNumber>(this->value));
				}
			} else {
				return defaultValue;
			}
		}

		template <typename T>
		typename std::enable_if<std::is_floating_point<T>::value, T>::type get(T defaultValue = 0.0f) const {
			if (this->type == LuaType::Number) {
				if (this->value.index() == 0) {
					return static_cast<T>(std::get<LuaInteger>(this->value));
				} else {
					return static_cast<T>(std::get<LuaNumber>(this->value));
				}
			} else {
				return defaultValue;
			}
		}

		template <typename T>
		typename std::enable_if<std::is_same<T, bool>::value, T>::type get(T defaultValue = false) const {
			if (this->type == LuaType::Boolean) {
				return static_cast<T>(std::get<LuaBoolean>(this->value));
			} else {
				return defaultValue;
			}
		}

		template <typename T>
		typename std::enable_if<std::is_same<T, std::string>::value, T>::type get(const T &defaultValue = "") const {
			if (this->type == LuaType::String) {
				return static_cast<T>(std::get<LuaString>(this->value));
			} else {
				return defaultValue;
			}
		}

		template <typename T>
		typename std::enable_if<std::is_same<T, LuaCFunction>::value, T>::type get(T defaultValue = nullptr) const {
			if (this->type == LuaType::Function) {
				return static_cast<T>(std::get<LuaCFunction>(this->value));
			} else {
				return defaultValue;
			}
		}

		template <typename T>
		operator T () {
			return this->get<T>();
		}

		template <typename T>
		static typename std::enable_if<std::is_integral<T>::value && !std::is_same<T, bool>::value, LuaValue>::type create(T value) {
			return LuaValue(LuaInteger(static_cast<lua_Integer>(value)));
		}

		template <typename T>
		static typename std::enable_if<std::is_floating_point<T>::value, LuaValue>::type create(T value) {
			return LuaValue(LuaNumber(static_cast<lua_Number>(value)));
		}

		template <typename T>
		static typename std::enable_if<std::is_same<T, bool>::value, LuaValue>::type create(T value) {
			return LuaValue(LuaBoolean(static_cast<bool>(value)));
		}

		template <typename T>
		static typename std::enable_if<std::is_same<T, std::string>::value, LuaValue>::type create(const T &value) {
			return LuaValue(LuaString(value));
		}

		template <typename T>
		static typename std::enable_if<std::is_same<T, LuaCFunction_ptr>::value, LuaValue>::type create(T value) {
			return LuaValue(LuaCFunction(value));
		}

		template <typename T>
		static typename std::enable_if<std::is_same<T, const char *>::value, LuaValue>::type create(T s) {
			return LuaValue(LuaString(s));
		}
	 private:
		LuaType type;
	 	std::variant<LuaInteger, LuaNumber, LuaBoolean, LuaString, LuaCFunction> value;
	};
}

#endif
