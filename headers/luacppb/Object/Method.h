#ifndef LUACPPB_OBJECT_METHOD_H_
#define LUACPPB_OBJECT_METHOD_H_

#include "luacppb/Base.h"
#include "luacppb/Invoke/Native.h"
#include "luacppb/Invoke/Method.h"
#include "luacppb/Core/Stack.h"
#include <memory>
#include <variant>
#include <optional>

namespace LuaCppB {

	template <typename C>
	class LuaCppObjectWrapper {
		using Raw = C *;
		using Unique = std::unique_ptr<C>;
		using Shared = std::shared_ptr<C>;
	 public:
	 	LuaCppObjectWrapper(Raw obj) : object(obj) {}
		LuaCppObjectWrapper(C &obj) : object(&obj) {}
	 	LuaCppObjectWrapper() {
		  this->object = std::unique_ptr<C>(reinterpret_cast<C *>(::operator new(sizeof(C))));
		}
		LuaCppObjectWrapper(Unique obj) : object(std::move(obj)) {}
		LuaCppObjectWrapper(Shared obj) : object(obj) {}
		~LuaCppObjectWrapper() {
			this->object = nullptr;
		}

		C *get() {
			switch (this->object.index()) {
				case 0:
				  return std::get<Raw>(object);
				case 1:
					return std::get<Unique>(object).get();
				case 2:
					return std::get<Shared>(object).get();
				default:
					return nullptr;
			}
		}
	 private:
		std::variant<Raw, Unique, Shared> object;
	};

	template <typename C, typename R, typename ... A>
	class LuaCppObjectMethodCall : public LuaData {
		using M = R (C::*)(A...);
	 public:
		LuaCppObjectMethodCall(M met) : method(met), className() {}
		LuaCppObjectMethodCall(M met, const std::string &cName) : method(met), className(cName) {}

		void push(lua_State *state) const override {
			LuaCppObjectMethodCallDescriptor<M> *descriptor = reinterpret_cast<LuaCppObjectMethodCallDescriptor<M> *>(
        lua_newuserdata(state, sizeof(LuaCppObjectMethodCallDescriptor<M>)));
			descriptor->method = this->method;
			if (this->className.has_value()) {
				lua_pushstring(state, this->className.value().c_str());
				lua_pushcclosure(state, &LuaCppObjectMethodCall<C, R, A...>::class_method_closure, 2);
			} else {
				lua_pushcclosure(state, &LuaCppObjectMethodCall<C, R, A...>::object_method_closure, 1);
			}
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

		static int object_method_closure(lua_State *state) {
			LuaStack stack(state);
			LuaCppObjectMethodCallDescriptor<M> *descriptor = stack.toUserData<LuaCppObjectMethodCallDescriptor<M> *>(lua_upvalueindex(1));
			LuaCppObjectWrapper<C> *object = stack.toPointer<LuaCppObjectWrapper<C> *>(1);
			return LuaCppObjectMethodCall<C, R, A...>::call(object->get(), descriptor->method, state);
		};

		static int class_method_closure(lua_State *state) {
			LuaStack stack(state);
			LuaCppObjectMethodCallDescriptor<M> *descriptor = stack.toUserData<LuaCppObjectMethodCallDescriptor<M> *>(lua_upvalueindex(1));
			std::string className = stack.toString(lua_upvalueindex(2));
			LuaCppObjectWrapper<C> *object = reinterpret_cast<LuaCppObjectWrapper<C> *>(const_cast<void *>(luaL_checkudata(state, 1, className.c_str())));
			if (object) {
				return LuaCppObjectMethodCall<C, R, A...>::call(object->get(), descriptor->method, state);
			} else {
				return 0;
			}
		};

		M method;
		std::optional<std::string> className;
	};
}

#endif