#ifndef LUACPPB_REFERENCE_BASE_H_
#define LUACPPB_REFERENCE_BASE_H_

#include "luacppb/Base.h"
#include "luacppb/Value/Value.h"
#include "luacppb/Value/Native.h"
#include "luacppb/Core/Runtime.h"
#include <optional>
#include <type_traits>
#include <functional>

namespace LuaCppB {

	class LuaReference {
	 public:
	 	LuaReference(LuaCppRuntime &runtime) : runtime(runtime) {}
		virtual ~LuaReference() = default;
		lua_State *getState();
		LuaCppRuntime &getRuntime() {
			return this->runtime;
		}

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
		typename std::enable_if<!std::is_base_of<LuaData, T>::value && !LuaValue::is_constructible<T>()>::type set(T &value) {
			this->setValue([&](lua_State *state) {
				LuaNativeValue::push<T>(state, this->runtime, value);
			});
		}

		template <typename T>
		typename std::enable_if<LuaValue::is_constructible<T>()>::type set(T &value) {
			LuaValue val = LuaValue::create<T>(value);
			this->setValue([&val](lua_State *state) {
				val.push(state);
			});
		}
	 protected:
		LuaCppRuntime &runtime;
	};
}

#endif