#ifndef LUACPPB_REFERENCE_HANDLE_H_
#define LUACPPB_REFERENCE_HANDLE_H_

#include "luacppb/Reference/Base.h"
#include "luacppb/Value/Native.h"
#include "luacppb/Invoke/Native.h"
#include "luacppb/Invoke/Invoke.h"
#include <memory>
#include <utility>
#include <type_traits>

namespace LuaCppB {

	class LuaReferenceHandle {
	 public:
	 	LuaReferenceHandle() : state(nullptr), ref(nullptr) {}
		LuaReferenceHandle(lua_State *state, std::unique_ptr<Internal::LuaReference> ref) : state(state), ref(std::move(ref)) {}
		LuaReferenceHandle(const LuaReferenceHandle &);

		Internal::LuaReference &getReference() const;
		LuaCppRuntime &getRuntime() const;
		bool exists();
		LuaType getType();
		LuaReferenceHandle operator[](const std::string &);
		LuaReferenceHandle operator[](lua_Integer);
		LuaReferenceHandle &operator=(const LuaReferenceHandle &);
		LuaValue operator*();

		template <typename T>
		LuaReferenceHandle &operator=(T &value) {
			this->ref->set<T>(value);
			return *this;
		}

		template <typename T>
		LuaReferenceHandle &operator=(T &&value) {
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
		Internal::LuaFunctionCallResult operator()(A &... args) {
			return Internal::LuaFunctionInvoke::invoke<Internal::LuaReference, A...>(*this->ref, this->getRuntime(), args...);
		}

		template <typename ... A>
		Internal::LuaFunctionCallResult operator()(A &&... args) {
			return Internal::LuaFunctionInvoke::invoke<Internal::LuaReference, A...>(*this->ref, this->getRuntime(), args...);
		}
	 private:
	 	lua_State *state;
		std::unique_ptr<Internal::LuaReference> ref;
	};
}

#endif