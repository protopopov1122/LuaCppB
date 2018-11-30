#ifndef LUACPPB_REFERENCE_HANDLE_H_
#define LUACPPB_REFERENCE_HANDLE_H_

#include "luacppb/Reference/Base.h"
#include "luacppb/Function.h"
#include <memory>

namespace LuaCppB {

	class LuaReferenceHandle {
	 public:
	 	LuaReferenceHandle() : state(nullptr), ref(nullptr) {}
		LuaReferenceHandle(lua_State *state, std::unique_ptr<LuaReference> ref) : state(state), ref(std::move(ref)) {}
		LuaReferenceHandle(const LuaReferenceHandle &);

		LuaReference &getReference() const;
		bool exists();
		LuaType getType();
		LuaReferenceHandle operator[](const std::string &);
		LuaReferenceHandle operator[](lua_Integer);
		LuaReferenceHandle &operator=(LuaData &);
		LuaReferenceHandle &operator=(const LuaReferenceHandle &);

		template <typename R, typename ... A>
		LuaReferenceHandle &operator=(R (*fn)(A...)) {
			CFunctionCall call(fn);
			this->ref->set(call);
			return *this;
		}

		template <typename T>
		LuaReferenceHandle &operator=(T value) {
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
	 private:
	 	lua_State *state;
		std::unique_ptr<LuaReference> ref;
	};
}

#endif