#ifndef LUACPPB_VALUE_H_
#define LUACPPB_VALUE_H_

#include "luacppb/Base.h"
#include <memory>
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

	class LuaValueBase : public LuaData {
	};

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
		void push(lua_State *) const override;
		static LuaTableBase get(lua_State *, int = -1);

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

	 	lua_State *state;
		int ref;
	};

	class LuaValue : public LuaData {
	 public:
		LuaValue() : type(LuaType::Nil) {}
		LuaValue(LuaInteger i) : type(LuaType::Number), value(i) {}
		LuaValue(LuaNumber n) : type(LuaType::Number), value(n) {}
		LuaValue(LuaBoolean b) : type(LuaType::Boolean), value(b) {}
		LuaValue(const LuaString &s) : type(LuaType::String), value(s) {}
		LuaValue(LuaCFunction f) : type(LuaType::Function), value(f) {}
		LuaValue(LuaTableBase v) : type(LuaType::Table), value(v) {}

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
		typename std::enable_if<std::is_same<T, LuaTableBase>::value, T>::type get() const {
			if (this->type == LuaType::Table) {
				return std::get<LuaTableBase>(this->value);
			} else {
				return LuaTableBase();
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
	 	std::variant<LuaInteger, LuaNumber, LuaBoolean, LuaString, LuaCFunction, LuaTableBase> value;
	};
}

#endif
