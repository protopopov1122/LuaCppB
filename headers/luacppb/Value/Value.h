#ifndef LUACPPB_VALUE_H_
#define LUACPPB_VALUE_H_

#include "luacppb/Base.h"
#include "luacppb/Value/Types.h"
#include <memory>
#include <variant>
#include <type_traits>
#include <iostream>
#include <optional>

namespace LuaCppB {

	class LuaValue : public LuaData {
	 public:
		LuaValue() : type(LuaType::Nil) {}
		LuaValue(LuaInteger i) : type(LuaType::Number), value(i) {}
		LuaValue(LuaNumber n) : type(LuaType::Number), value(n) {}
		LuaValue(LuaBoolean b) : type(LuaType::Boolean), value(b) {}
		LuaValue(const LuaString &s) : type(LuaType::String), value(s) {}
		LuaValue(LuaCFunction f) : type(LuaType::Function), value(f) {}
		LuaValue(LuaTable t) : type(LuaType::Table), value(t) {}

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
		typename std::enable_if<std::is_same<T, LuaTable>::value, T>::type get() const {
			if (this->type == LuaType::Table) {
				return std::get<LuaTable>(this->value);
			} else {
				return LuaTable();
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
	 	std::variant<LuaInteger, LuaNumber, LuaBoolean, LuaString, LuaCFunction, LuaTable> value;
	};
}

#endif
