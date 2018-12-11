#ifndef LUACPPB_OBJECT_METHOD_H_
#define LUACPPB_OBJECT_METHOD_H_

#include "luacppb/Base.h"
#include "luacppb/Invoke/Native.h"
#include "luacppb/Invoke/Method.h"
#include <memory>
#include <variant>
#include <optional>

namespace LuaCppB {

	template <typename C>
	class CppObjectWrapper {
		using Raw = C *;
		using Unique = std::unique_ptr<C>;
		using Shared = std::shared_ptr<C>;
	 public:
	 	CppObjectWrapper(Raw obj) : object(obj) {}
		CppObjectWrapper(C &obj) : object(&obj) {}
	 	CppObjectWrapper() {
		  this->object = std::unique_ptr<C>(reinterpret_cast<C *>(::operator new(sizeof(C))));
		}
		CppObjectWrapper(Unique obj) : object(std::move(obj)) {}
		CppObjectWrapper(Shared obj) : object(obj) {}
		~CppObjectWrapper() {
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
	class CppObjectMethodCall : public LuaData {
		using M = R (C::*)(A...);
	 public:
		CppObjectMethodCall(M met) : method(met), className() {}
		CppObjectMethodCall(M met, const std::string &cName) : method(met), className(cName) {}

		void push(lua_State *state) const override {
			CppObjectMethodCallDescriptor<M> *descriptor = reinterpret_cast<CppObjectMethodCallDescriptor<M> *>(
        lua_newuserdata(state, sizeof(CppObjectMethodCallDescriptor<M>)));
			descriptor->method = this->method;
			if (this->className.has_value()) {
				lua_pushstring(state, this->className.value().c_str());
				lua_pushcclosure(state, &CppObjectMethodCall<C, R, A...>::class_method_closure, 2);
			} else {
				lua_pushcclosure(state, &CppObjectMethodCall<C, R, A...>::object_method_closure, 1);
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
			CppObjectMethodCallDescriptor<M> *descriptor = reinterpret_cast<CppObjectMethodCallDescriptor<M> *>(lua_touserdata(state, lua_upvalueindex(1)));
			CppObjectWrapper<C> *object = reinterpret_cast<CppObjectWrapper<C> *>(const_cast<void *>(lua_topointer(state, 1)));
			return CppObjectMethodCall<C, R, A...>::call(object->get(), descriptor->method, state);
		};

		static int class_method_closure(lua_State *state) {
			CppObjectMethodCallDescriptor<M> *descriptor = reinterpret_cast<CppObjectMethodCallDescriptor<M> *>(lua_touserdata(state, lua_upvalueindex(1)));
			const char *className = lua_tostring(state, lua_upvalueindex(2));
			CppObjectWrapper<C> *object = reinterpret_cast<CppObjectWrapper<C> *>(const_cast<void *>(luaL_checkudata(state, 1, className)));
			if (object) {
				return CppObjectMethodCall<C, R, A...>::call(object->get(), descriptor->method, state);
			} else {
				return 0;
			}
		};

		M method;
		std::optional<std::string> className;
	};
}

#endif