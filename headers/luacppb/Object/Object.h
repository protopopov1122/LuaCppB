#ifndef LUACPPB_OBJECT_OBJECT_H_
#define LUACPPB_OBJECT_OBJECT_H_

#include "luacppb/Base.h"
#include "luacppb/Invoke/Native.h"
#include <map>

namespace LuaCppB {

  template <typename M>
  struct CppObjectMethodCallDescriptor {
    M method;
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
      C *object = reinterpret_cast<C *>(const_cast<void *>(lua_topointer(state, 1)));
			return CppObjectMethodCall<C, R, A...>::call(object, descriptor->method, state);
		};

		M method;
	};

  template <typename T>
  class LuaCppObject : public LuaData {
   public:
    LuaCppObject(T *obj) : object(obj) {}
    LuaCppObject(T &obj) : object(&obj) {}

    template <typename R, typename ... A>
    void bind(const std::string &key, R (T::*method_ptr)(A...)) {
      this->methods[key] = std::make_shared<CppObjectMethodCall<T, R, A...>>(method_ptr);
    }

    void push(lua_State *state) const override {
      lua_pushlightuserdata(state, this->object);
      lua_newtable(state);
      lua_newtable(state);
      for (auto it = this->methods.begin(); it != this->methods.end(); ++it) {
        it->second->push(state);
        lua_setfield(state, -2, it->first.c_str());
      }
      lua_setfield(state, -2, "__index");
      lua_setmetatable(state, -2);
    }
   private:
    T *object;
    std::map<std::string, std::shared_ptr<LuaData>> methods;
  };
}

#endif