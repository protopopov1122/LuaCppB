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

	typedef int (*LuaCFunction)(lua_State *);

	class LuaData {
	 public:
		virtual ~LuaData() = default;
		virtual void push(lua_State *state) const = 0;
	};

	class LuaValue : public LuaData {
	 public:
		LuaValue() : type(LuaType::Nil) {}
		LuaValue(lua_Integer i) : type(LuaType::Number), value(i) {}
		LuaValue(lua_Number n) : type(LuaType::Number), value(n) {}
		LuaValue(bool b) : type(LuaType::Boolean), value(b) {}
		LuaValue(const std::string &s) : type(LuaType::String), value(std::string(s)) {}
		LuaValue(LuaCFunction f) : type(LuaType::Function), value(f) {}

		LuaType getType() const;
		void push(lua_State *state) const override;
		static std::optional<LuaValue> peek(lua_State *, lua_Integer = -1);

		template <typename T>
		typename std::enable_if<std::is_integral<T>::value && !std::is_same<T, bool>::value, T>::type get(T defaultValue = 0) const {
			if (this->type == LuaType::Number) {
				if (this->value.index() == 0) {
					return static_cast<T>(std::get<lua_Integer>(this->value));
				} else {
					return static_cast<T>(std::get<lua_Number>(this->value));
				}
			} else {
				return defaultValue;
			}
		}

		template <typename T>
		typename std::enable_if<std::is_floating_point<T>::value, T>::type get(T defaultValue = 0.0f) const {
			if (this->type == LuaType::Number) {
				if (this->value.index() == 0) {
					return static_cast<T>(std::get<lua_Integer>(this->value));
				} else {
					return static_cast<T>(std::get<lua_Number>(this->value));
				}
			} else {
				return defaultValue;
			}
		}

		template <typename T>
		typename std::enable_if<std::is_same<T, bool>::value, T>::type get(T defaultValue = false) const {
			if (this->type == LuaType::Boolean) {
				return static_cast<T>(std::get<bool>(this->value));
			} else {
				return defaultValue;
			}
		}

		template <typename T>
		typename std::enable_if<std::is_same<T, std::string>::value, T>::type get(const T &defaultValue = "") const {
			if (this->type == LuaType::String) {
				return static_cast<T>(std::get<std::string>(this->value));
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
			return LuaValue(static_cast<lua_Integer>(value));
		}

		template <typename T>
		static typename std::enable_if<std::is_floating_point<T>::value, LuaValue>::type create(T value) {
			return LuaValue(static_cast<lua_Number>(value));
		}

		template <typename T>
		static typename std::enable_if<std::is_same<T, bool>::value, LuaValue>::type create(T value) {
			return LuaValue(static_cast<bool>(value));
		}

		template <typename T>
		static typename std::enable_if<std::is_same<T, std::string>::value, LuaValue>::type create(const T &value) {
			return LuaValue(value);
		}

		template <typename T>
		static typename std::enable_if<std::is_same<T, LuaCFunction>::value, LuaValue>::type create(T value) {
			return LuaValue(value);
		}

		template <typename T>
		static typename std::enable_if<std::is_same<T, const char *>::value, LuaValue>::type create(T s) {
			return LuaValue(std::string(s));
		}
	 private:
		LuaType type;
	 	std::variant<lua_Integer, lua_Number, bool, std::string, LuaCFunction> value;
	};
}

#endif
