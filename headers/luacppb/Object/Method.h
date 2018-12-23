#ifndef LUACPPB_OBJECT_METHOD_H_
#define LUACPPB_OBJECT_METHOD_H_

#include "luacppb/Base.h"
#include "luacppb/Object/Wrapper.h"
#include "luacppb/Invoke/Native.h"
#include "luacppb/Invoke/Method.h"
#include "luacppb/Core/Stack.h"
#include "luacppb/Core/Runtime.h"
#include <optional>

namespace LuaCppB {

	template <typename C, typename M, typename R, typename ... A>
	class LuaCppObjectMethodCall : public LuaData {
	 public:
		LuaCppObjectMethodCall(M met, LuaCppRuntime &runtime) : method(met), className(), runtime(runtime) {}
		LuaCppObjectMethodCall(M met, const std::string &cName, LuaCppRuntime &runtime) : method(met), className(cName), runtime(runtime) {}

		void push(lua_State *state) const override {
			LuaCppObjectMethodCallDescriptor<M> *descriptor = reinterpret_cast<LuaCppObjectMethodCallDescriptor<M> *>(
        lua_newuserdata(state, sizeof(LuaCppObjectMethodCallDescriptor<M>)));
			descriptor->method = this->method;
			lua_pushlightuserdata(state, reinterpret_cast<void *>(&this->runtime));
			if (this->className.has_value()) {
				lua_pushstring(state, this->className.value().c_str());
				lua_pushcclosure(state, &LuaCppObjectMethodCall<C, M, R, A...>::class_method_closure, 3);
			} else {
				lua_pushcclosure(state, &LuaCppObjectMethodCall<C, M, R, A...>::object_method_closure, 2);
			}
		}
	 private:
	 	static int call(C *object, M method, LuaCppRuntime &runtime, lua_State *state) {
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
				return NativeFunctionResult<R>::set(state, runtime, result);
			}
		};

		static int object_method_closure(lua_State *state) {
			LuaStack stack(state);
			LuaCppObjectMethodCallDescriptor<M> *descriptor = stack.toUserData<LuaCppObjectMethodCallDescriptor<M> *>(lua_upvalueindex(1));
			LuaCppRuntime &runtime = *stack.toPointer<LuaCppRuntime *>(lua_upvalueindex(2));
			LuaCppObjectWrapper<C> *object = stack.toPointer<LuaCppObjectWrapper<C> *>(1);
			return LuaCppObjectMethodCall<C, M, R, A...>::call(object->get(), descriptor->method, runtime, state);
		};

		static int class_method_closure(lua_State *state) {
			LuaStack stack(state);
			LuaCppObjectMethodCallDescriptor<M> *descriptor = stack.toUserData<LuaCppObjectMethodCallDescriptor<M> *>(lua_upvalueindex(1));
			LuaCppRuntime &runtime = *stack.toPointer<LuaCppRuntime *>(lua_upvalueindex(2));
			std::string className = stack.toString(lua_upvalueindex(3));
			LuaCppObjectWrapper<C> *object = reinterpret_cast<LuaCppObjectWrapper<C> *>(const_cast<void *>(luaL_checkudata(state, 1, className.c_str())));
			if (object) {
				return LuaCppObjectMethodCall<C, M, R, A...>::call(object->get(), descriptor->method, runtime, state);
			} else {
				return 0;
			}
		};

		M method;
		std::optional<std::string> className;
		LuaCppRuntime &runtime;
	};
}

#endif