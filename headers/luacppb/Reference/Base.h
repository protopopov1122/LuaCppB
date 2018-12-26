#ifndef LUACPPB_REFERENCE_BASE_H_
#define LUACPPB_REFERENCE_BASE_H_

#include "luacppb/Base.h"
#include "luacppb/Value/Value.h"
#include "luacppb/Value/Native.h"
#include "luacppb/Invoke/Coro.h"
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
		typename std::enable_if<std::is_convertible<LuaValue, T>::value && !std::is_same<T, LuaValue>::value && !std::is_same<T, LuaCoroutine>::value, T>::type
			get() {
			return this->get<LuaValue>().get<T>();
		}

		template <typename T>
		typename std::enable_if<std::is_same<T, LuaCoroutine>::value, T>::type
			get() {
			LuaCoroutine coro(this->getRuntime());
			this->putOnTop([&](lua_State *state) {
				if (lua_isthread(state, -1)) {
					coro = LuaCoroutine(LuaThread(state, -1), this->getRuntime());
				} else {
					coro = LuaCoroutine(state, -1, this->getRuntime());
				}
			});
			return coro;
		}

		template <typename T>
		typename std::enable_if<std::is_base_of<LuaData, T>::value>::type set(T value) {
			this->setValue([&value](lua_State *state) {
				value.push(state);
			});
		}

		template <typename T>
		typename std::enable_if<!std::is_base_of<LuaData, T>::value>::type set(T &value) {
			this->setValue([&](lua_State *state) {
				LuaNativeValue::push<T>(state, this->runtime, value);
			});
		}
	 protected:
		LuaCppRuntime &runtime;
	};
}

#endif