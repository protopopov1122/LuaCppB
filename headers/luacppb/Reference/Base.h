#ifndef LUACPPB_REFERENCE_BASE_H_
#define LUACPPB_REFERENCE_BASE_H_

#include "luacppb/Base.h"
#include "luacppb/Value/Value.h"
#include <optional>
#include <type_traits>
#include <functional>

namespace LuaCppB {

	class LuaReference {
	 public:
		virtual ~LuaReference() = default;
		lua_State *getState();

		virtual bool putOnTop(std::function<void (lua_State *)>) = 0;
		virtual bool setValue(std::function<void (lua_State *)>) = 0;

		template <typename T>
		typename std::enable_if<std::is_same<T, LuaValue>::value, T>::type get() {
			std::optional<LuaValue> value;
			this->putOnTop([&](lua_State *state) {
				value = LuaValue::peek(state);
			});
			return value.value_or(LuaValue());
		}

		template <typename T>
		typename std::enable_if<std::is_convertible<LuaValue, T>::value && !std::is_same<T, LuaValue>::value, T>::type get() {
			return this->get<LuaValue>().get<T>();
		}

		template <typename T>
		typename std::enable_if<std::is_base_of<LuaData, T>::value>::type set(T value) {
			this->setValue([&value](lua_State *state) {
				value.push(state);
			});
		}

		void set(LuaData &value) {
			this->setValue([&value](lua_State *state) {
				value.push(state);
			});
		}

		template <typename T>
		typename std::enable_if<!std::is_base_of<LuaData, T>::value>::type set(T value) {
			LuaValue val = LuaValue::create<T>(value);
			this->setValue([&val](lua_State *state) {
				val.push(state);
			});
		}
	};
}

#endif