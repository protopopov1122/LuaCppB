#ifndef LUACPPB_OBJECT_METHOD_H_
#define LUACPPB_OBJECT_METHOD_H_

#include "luacppb/Base.h"
#include "luacppb/Value/Native.h"
#include "luacppb/Object/Wrapper.h"
#include "luacppb/Invoke/ArgRet.h"
#include "luacppb/Invoke/Method.h"
#include "luacppb/Core/Stack.h"
#include "luacppb/Core/Runtime.h"
#include <optional>

namespace LuaCppB::Internal {

	template <typename C, typename M, typename R, typename ... A>
	class LuaCppObjectMethodCall : public LuaData {
	 public:
		LuaCppObjectMethodCall(M met, const std::string &cName, LuaCppRuntime &runtime) : method(met), className(cName), runtime(runtime) {}

		void push(lua_State *state) const override {
			Internal::LuaStack stack(state);
			LuaCppObjectMethodCallDescriptor<M> *descriptor = stack.push<LuaCppObjectMethodCallDescriptor<M>>();
			descriptor->method = this->method;
			stack.push(&this->runtime);
			stack.push(this->className);
			stack.push(&LuaCppObjectMethodCall<C, M, R, A...>::class_method_closure, 3);
		}
	 private:
	 	static int call(C *object, M method, LuaCppRuntime &runtime, lua_State *state) {
			std::tuple<A...> args = NativeFunctionArgumentsTuple<2, A...>::value(state, runtime);
			if constexpr (std::is_void<R>::value) {
				std::apply([object, method](A... args) {	
					return (object->*method)(args...);
				}, args);
				return 0;
			} else {
				return NativeFunctionResult<Internal::LuaNativeValue, R>::set(state, runtime, std::apply([object, method](A... args) {	
					return (object->*method)(args...);
				}, args));
			}
		};

		static int class_method_closure(lua_State *state) {
			try {
				Internal::LuaStack stack(state);
				LuaCppObjectMethodCallDescriptor<M> *descriptor = stack.toUserData<LuaCppObjectMethodCallDescriptor<M> *>(lua_upvalueindex(1));
				LuaCppRuntime &runtime = *stack.toPointer<LuaCppRuntime *>(lua_upvalueindex(2));
				std::string className = stack.toString(lua_upvalueindex(3));
				LuaCppObjectWrapper<C> *object = checkUserData(className, state, 1);
				if (object) {
					return LuaCppObjectMethodCall<C, M, R, A...>::call(object->get(), descriptor->method, runtime, state);
				} else {
					return 0;
				}
			} catch (std::exception &ex) {
				return luacpp_handle_exception(state, std::current_exception());
			}
		};

		static LuaCppObjectWrapper<C> *checkUserData(const std::string &className, lua_State *state, int index = -1) {
			Internal::LuaStack stack(state);
			if (!stack.is<LuaType::UserData>(index)) {
				return nullptr;
			}
			lua_getmetatable(state, index);
			stack.push(std::string("__name"));
			lua_rawget(state, -2);
			std::string name = stack.toString(-1);
			stack.pop(2);
			if (name.find(className) == 0) {
				return stack.toPointer<LuaCppObjectWrapper<C> *>(index);
			} else {
				return nullptr;
			}
		}

		M method;
		std::string className;
		LuaCppRuntime &runtime;
	};
}

#endif