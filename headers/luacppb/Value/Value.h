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

	class LuaValue : public LuaData {
	 public:
		LuaValue() : type(LuaType::Nil) {}
		LuaValue(LuaInteger i) : type(LuaType::Number), value(i) {}
		LuaValue(LuaNumber n) : type(LuaType::Number), value(n) {}
		LuaValue(LuaBoolean b) : type(LuaType::Boolean), value(b) {}
		LuaValue(const LuaString &s) : type(LuaType::String), value(s) {}
		LuaValue(LuaFunction f) : type(LuaType::Function), value(f) {}
		LuaValue(LuaCFunction f) : type(LuaType::Function), value(f) {}
		LuaValue(LuaTable t) : type(LuaType::Table), value(t) {}
		LuaValue(void *ptr) : type(LuaType::LightUserData), value(ptr) {}
		LuaValue(LuaUserData ud) : type(LuaType::UserData), value(ud) {}
		LuaValue(LuaThread th) : type(LuaType::Thread), value(th) {}

		LuaType getType() const noexcept;
		void push(lua_State *state) const override;
		static std::optional<LuaValue> peek(lua_State *, lua_Integer = -1);

		template <typename T>
		typename std::enable_if<std::is_integral<T>::value && !std::is_same<T, bool>::value, T>::type get(T defaultValue = 0) const {
			if (this->type == LuaType::Number) {
				assert(this->value.index() == 0 || this->value.index() == 1);
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
				assert(this->value.index() == 0 || this->value.index() == 1);
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
				assert(this->value.index() == 2);
				return static_cast<T>(std::get<LuaBoolean>(this->value));
			} else {
				return defaultValue;
			}
		}

		template <typename T>
		typename std::enable_if<std::is_same<T, std::string>::value, T>::type get(const T &defaultValue = "") const {
			if (this->type == LuaType::String) {
				assert(this->value.index() == 3);
				return static_cast<T>(std::get<LuaString>(this->value));
			} else {
				return defaultValue;
			}
		}

		template <typename T>
		typename std::enable_if<std::is_same<T, LuaCFunction>::value, T>::type get(T defaultValue = nullptr) const {
			if (this->type == LuaType::Function) {
				if (this->value.index() == 4) {
					return static_cast<T>(std::get<LuaCFunction>(this->value));
				} else {
					assert(this->value.index() == 9);
					return std::get<LuaFunction>(this->value).toCFunction();
				}
			} else {
				return defaultValue;
			}
		}

		template <typename T>
		typename std::enable_if<std::is_same<T, LuaTable>::value, T>::type get() const {
			if (this->type == LuaType::Table) {
				assert(this->value.index() == 5);
				return std::get<LuaTable>(this->value);
			} else {
				return LuaTable();
			}
		}

		template <typename T>
		typename std::enable_if<std::is_same<T, LuaUserData>::value, T>::type get() const {
			if (this->type == LuaType::UserData) {
				assert(this->value.index() == 7);
				return std::get<LuaUserData>(this->value);
			} else {
				return LuaUserData();
			}
		}

		template <typename T>
		typename std::enable_if<std::is_pointer<T>::value && std::is_class<typename std::remove_pointer<T>::type>::value, T>::type get(T defaultValue = nullptr) const {
			using V = typename std::remove_pointer<T>::type;
			if (this->type == LuaType::UserData) {
				assert(this->value.index() == 7);
				LuaCppObjectWrapper<V> *ptr = std::get<LuaUserData>(this->value).toPointer<LuaCppObjectWrapper<V> *>();
				return ptr->get();
			} else {
				return defaultValue;
			}
		}

		template <typename T>
		typename std::enable_if<std::is_reference<T>::value && std::is_class<typename std::remove_reference<T>::type>::value, T>::type get() const {
			using V = typename std::remove_reference<T>::type;
			if (this->type == LuaType::UserData) {
				assert(this->value.index() == 7);
				LuaCppObjectWrapper<V> *ptr = std::get<LuaUserData>(this->value).toPointer<LuaCppObjectWrapper<V> *>();
				if (ptr != nullptr) {
					return *ptr->get();
				} else {
					throw LuaCppBError("Null pointer can't be dereferenced", LuaCppBErrorCode::NullPointerDereference);
				}
			} else {
				throw LuaCppBError("Type casting failed", LuaCppBErrorCode::IncorrectTypeCast);
			}
		}

		template <typename T>
		typename std::enable_if<std::is_same<T, LuaThread>::value, T>::type get() const {
			if (this->type == LuaType::Thread) {
				assert(this->value.index() == 8);
				return std::get<LuaThread>(this->value);
			} else {
				return LuaThread();
			}
		}

		template <typename T>
		typename std::enable_if<std::is_same<T, LuaFunction>::value, T>::type get() const {
			if (this->type == LuaType::Function) {
				assert(this->value.index() == 9);
				return std::get<LuaFunction>(this->value);
			} else {
				return LuaFunction();
			}
		}

		template <typename T>
		typename std::enable_if<std::is_enum<T>::value, T>::type get() const {
			using EnumType = typename std::underlying_type<T>::type;
			return static_cast<T>(this->get<EnumType>());
		}

		template <typename T>
		operator T () {
			return this->get<T>();
		}

		template <typename T>
		static typename std::enable_if<std::is_integral<T>::value && !std::is_same<T, bool>::value, LuaValue>::type create(T value) noexcept {
			return LuaValue(LuaInteger(static_cast<lua_Integer>(value)));
		}

		template <typename T>
		static typename std::enable_if<std::is_floating_point<T>::value, LuaValue>::type create(T value) noexcept {
			return LuaValue(LuaNumber(static_cast<lua_Number>(value)));
		}

		template <typename T>
		static typename std::enable_if<std::is_same<T, bool>::value, LuaValue>::type create(T value) noexcept {
			return LuaValue(LuaBoolean(static_cast<bool>(value)));
		}

		template <typename T>
		static typename std::enable_if<std::is_same<T, std::string>::value, LuaValue>::type create(const T &value) noexcept {
			return LuaValue(LuaString(value));
		}

		template <typename T>
		static typename std::enable_if<std::is_same<T, LuaCFunction>::value, LuaValue>::type create(T value) noexcept {
			return LuaValue(value);
		}

		template <typename T>
		static typename std::enable_if<std::is_same<T, const char *>::value, LuaValue>::type create(T s) noexcept {
			return LuaValue(LuaString(s));
		}

		template <typename T>
		static typename std::enable_if<std::is_array<T>::value && std::is_same<typename std::remove_extent<T>::type, const char>::value, LuaValue>::type
			create(T s) noexcept {
			return LuaValue(LuaString(s));
		}

		template <typename T>
		static constexpr bool is_constructible() noexcept {
			return std::is_integral<T>::value ||
				std::is_floating_point<T>::value ||
				std::is_same<T, bool>::value ||
				std::is_same<T, std::string>::value ||
				std::is_same<T, LuaCFunction>::value ||
				std::is_same<T, const char *>::value ||
				std::is_array<T>::value && std::is_same<typename std::remove_extent<T>::type, const char>::value;
		}
	 private:
		LuaType type;
	 	std::variant<LuaInteger, LuaNumber, LuaBoolean, LuaString, LuaCFunction, LuaTable, void *, LuaUserData, LuaThread, LuaFunction> value;
	};
}

#endif
