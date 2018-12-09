#ifndef LUACPPB_OBJECT_METHOD_H_
#define LUACPPB_OBJECT_METHOD_H_

#include "luacppb/Base.h"
#include "luacppb/Invoke/Native.h"
#include "luacppb/Invoke/Method.h"

namespace LuaCppB {

	template <typename C>
	class CppObjectWrapper {
	 public:
	 	CppObjectWrapper(C *obj) : object(obj), owner(false) {}
	 	CppObjectWrapper() : owner(true) {
		  this->object = reinterpret_cast<C *>(::operator new(sizeof(C)));
		}
		~CppObjectWrapper() {
			if (this->owner) {
				this->object->~C();
				::operator delete(this->object);
			}
			this->object = nullptr;
		}

		C *get() {
			return this->object;
		}
	 private:
		C *object;
		bool owner;
	};

	template <typename C, typename R, typename ... A>
	class CppObjectMethodCall : public LuaData {
		using M = R (C::*)(A...);
	 public:
		CppObjectMethodCall(M met) : method(met) {}
		void push(lua_State *state) const override {
			CppObjectMethodCallDescriptor<M> *descriptor = reinterpret_cast<CppObjectMethodCallDescriptor<M> *>(
        lua_newuserdata(state, sizeof(CppObjectMethodCallDescriptor<M>)));
			descriptor->method = this->method;
			lua_pushcclosure(state, &CppObjectMethodCall<C, R, A...>::method_closure, 1);	
		}
	 private:
	 	static int call(C *object, M method, lua_State *state) {
			std::tuple<A...> args = NativeFunctionArgumentsTuple<2, A...>::value(state);
			if constexpr (std::is_void<R>::value) {
				std::apply([object, method](A... args) {	
					return (object->*method)(args...);
				}, args);
				return 0;
			} else {
				R result = std::apply([object, method](A... args) {	
					return (object->*method)(args...);
				}, args);
				NativeFunctionResult<R>::set(state, result);
				return 1;
			}
		};

		static int method_closure(lua_State *state) {
			CppObjectMethodCallDescriptor<M> *descriptor = reinterpret_cast<CppObjectMethodCallDescriptor<M> *>(lua_touserdata(state, lua_upvalueindex(1)));
			CppObjectWrapper<C> *object = reinterpret_cast<CppObjectWrapper<C> *>(const_cast<void *>(lua_topointer(state, 1)));
			return CppObjectMethodCall<C, R, A...>::call(object->get(), descriptor->method, state);
		};

		M method;
	};
}

#endif