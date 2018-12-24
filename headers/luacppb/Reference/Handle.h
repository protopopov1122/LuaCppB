#ifndef LUACPPB_REFERENCE_HANDLE_H_
#define LUACPPB_REFERENCE_HANDLE_H_

#include "luacppb/Reference/Base.h"
#include "luacppb/Invoke/Native.h"
#include "luacppb/Invoke/Lua.h"
#include "luacppb/Invoke/Coro.h"
#include <memory>
#include <utility>
#include <type_traits>

namespace LuaCppB {

	class LuaReferenceHandle {
	 public:
	 	LuaReferenceHandle() : state(nullptr), ref(nullptr) {}
		LuaReferenceHandle(lua_State *state, std::unique_ptr<LuaReference> ref) : state(state), ref(std::move(ref)) {}
		LuaReferenceHandle(const LuaReferenceHandle &);

		LuaReference &getReference() const;
		LuaCppRuntime &getRuntime() const;
		bool exists();
		LuaType getType();
		LuaReferenceHandle operator[](const std::string &);
		LuaReferenceHandle operator[](lua_Integer);
		LuaReferenceHandle &operator=(LuaData &);
		LuaReferenceHandle &operator=(const LuaReferenceHandle &);
		LuaValue operator*();
		
		template <typename R, typename ... A>
		LuaReferenceHandle &operator=(R (*fn)(A...)) {
			NativeFunctionCall call(fn, this->getRuntime());
			this->ref->set(call);
			return *this;
		}

		template <typename T>
		typename std::enable_if<std::is_base_of<LuaData, T>::value || LuaValue::is_constructible<T>(), LuaReferenceHandle>::type
			&operator=(T &value) {
			this->ref->set<T>(value);
			return *this;
		}

		template <typename T>
		typename std::enable_if<(std::is_base_of<LuaData, T>::value || LuaValue::is_constructible<T>()) && !std::is_reference<T>::value, LuaReferenceHandle>::type
			&operator=(T &&value) {
			this->ref->set<T>(value);
			return *this;
		}

		template <typename T>
		typename std::enable_if<!std::is_base_of<LuaData, T>::value && !LuaValue::is_constructible<T>(), LuaReferenceHandle>::type
			&operator=(T &value) {
			this->ref->set<T>(value);
			return *this;
		}

		template <typename T>
		typename std::enable_if<!std::is_base_of<LuaData, T>::value && !LuaValue::is_constructible<T>() && !std::is_reference<T>::value, LuaReferenceHandle>::type
			&operator=(T &&value) {
			this->ref->set<T>(value);
			return *this;
		}

		template <typename T>
		operator T () {
			return this->ref->get<T>();
		}

		template <typename T>
		T get() {
			return this->ref->get<T>();
		}

		template <typename ... A>
		LuaFunctionCallResult operator()(A &... args) {
			std::vector<LuaValue> results;
			LuaStatusCode status = LuaStatusCode::Ok;
			this->ref->putOnTop([&](lua_State *state) {
				if (lua_isfunction(state, -1)) {
					status = LuaFunctionCall::call<A...>(state, -1, this->getRuntime(), results, args...);
				} else if (lua_isthread(state, -1)) {
					LuaCoroutine coro(LuaThread(state, -1), this->getRuntime());
					status = coro.call(results, args...);
				}
			});
			return LuaFunctionCallResult(results, status);
		}

		template <typename ... A>
		LuaFunctionCallResult operator()(A &&... args) {
			std::vector<LuaValue> results;
			LuaStatusCode status = LuaStatusCode::Ok;
			this->ref->putOnTop([&](lua_State *state) {
				if (lua_isfunction(state, -1)) {
					status = LuaFunctionCall::call<A...>(state, -1, this->getRuntime(), results, args...);
				} else if (lua_isthread(state, -1)) {
					LuaCoroutine coro(LuaThread(state, -1), this->getRuntime());
					status = coro.call(results, args...);
				}
			});
			return LuaFunctionCallResult(results, status);
		}
	 private:
	 	lua_State *state;
		std::unique_ptr<LuaReference> ref;
	};
}

#endif