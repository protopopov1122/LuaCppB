#ifndef LUACPPB_OBJECT_CLASS_H_
#define LUACPPB_OBJECT_CLASS_H_

#include "luacppb/Base.h"
#include "luacppb/Object/Method.h"
#include "luacppb/Core/Stack.h"
#include <map>
#include <type_traits>

namespace LuaCppB {

	template <typename C, typename ... A>
	class LuaCppObjectInitializer : public LuaData {
		using F = void (*)(C *, A...);
	 public:
		LuaCppObjectInitializer(const std::string &className, F fn) : className(className), function(fn) {}

		void push(lua_State *state) const override {
			lua_pushlightuserdata(state, reinterpret_cast<void *>(this->function));
      lua_pushstring(state, this->className.c_str());
			lua_pushcclosure(state, LuaCppObjectInitializer<C, A...>::function_closure, 2);
		}
	 private:
		static int call(F function, const std::string &className, lua_State *state) {
			std::tuple<A...> args = NativeFunctionArgumentsTuple<1, A...>::value(state);
      LuaCppObjectWrapper<C> *object = reinterpret_cast<LuaCppObjectWrapper<C> *>(lua_newuserdata(state, sizeof(LuaCppObjectWrapper<C>)));
      new(object) LuaCppObjectWrapper<C>();
      std::tuple<C *, A...> allArgs = std::tuple_cat(std::make_tuple(object->get()), args);
      std::apply(function, allArgs);
      luaL_setmetatable(state, className.c_str());
      return 1;
		};

		static int function_closure(lua_State *state) {
      LuaStack stack(state);
			F fn = stack.toPointer<F>(lua_upvalueindex(1));
      std::string className = stack.toString(lua_upvalueindex(2));
			return LuaCppObjectInitializer<C, A...>::call(fn, className, state);
		};

    std::string className;
		F function;
	};

  template <typename C>
  class LuaCppClass : public LuaData {
   public:
    LuaCppClass(const std::string &className)
      : className(className) {}

    const std::string &getClassName() const {
      return this->className;
    }
    
    void push(lua_State *state) const override {
      if (luaL_newmetatable(state, this->className.c_str()) != 0) {
        lua_newtable(state);
        for (auto it = this->methods.begin(); it != this->methods.end(); ++it) {
          it->second->push(state);
          lua_setfield(state, -2, it->first.c_str());
        }
        lua_setfield(state, -2, "__index");
        if constexpr (std::is_default_constructible<C>::value) {
          lua_pushstring(state, this->className.c_str());
          lua_pushcclosure(state, &LuaCppClass<C>::newObject, 1);
          lua_setfield(state, -2, "new");
        }
        lua_pushstring(state, this->className.c_str());
        lua_pushcclosure(state, &LuaCppClass<C>::gcObject, 1);
        lua_setfield(state, -2, "__gc");
        for (auto it = this->initializers.begin(); it != this->initializers.end(); ++it) {
          it->second->push(state);
          lua_setfield(state, -2, it->first.c_str());
        }
      }
    }

    template <typename R, typename ... A>
    void bind(const std::string &key, R (C::*method)(A...)) {
      this->methods[key] = std::make_shared<LuaCppObjectMethodCall<C, R, A...>>(NativeMethodWrapper(method).get(), this->className);
    }

    template <typename R, typename ... A>
    void bind(const std::string &key, R (C::*method)(A...) const) {
      this->methods[key] = std::make_shared<LuaCppObjectMethodCall<C, R, A...>>(NativeMethodWrapper(method).get(), this->className);
    }

    template <typename ... A>
    void initializer(const std::string &key, void (*initializer)(C *, A...)) {
      this->initializers[key] = std::make_shared<LuaCppObjectInitializer<C, A...>>(this->className, initializer);
    }
   private:
    static int newObject(lua_State *state) {
      if constexpr (std::is_default_constructible<C>::value) {
        LuaStack stack(state);
        std::string className = stack.toString(lua_upvalueindex(1));
        LuaCppObjectWrapper<C> *object = reinterpret_cast<LuaCppObjectWrapper<C> *>(lua_newuserdata(state, sizeof(LuaCppObjectWrapper<C>)));
        new(object) LuaCppObjectWrapper<C>();
        new(object->get()) C();
        luaL_setmetatable(state, className.c_str());
      } else {
        lua_pushnil(state);
      }
      return 1;
    }

    static int gcObject(lua_State *state) {
      LuaStack stack(state);
      std::string className = stack.toString(lua_upvalueindex(1));
      LuaCppObjectWrapper<C> *object = reinterpret_cast<LuaCppObjectWrapper<C> *>(luaL_checkudata(state, 1, className.c_str()));
      if (object) {
        object->~LuaCppObjectWrapper();
        ::operator delete(object, object);
      }
      return 0;
    }

    std::string className;
    std::map<std::string, std::shared_ptr<LuaData>> methods;
    std::map<std::string, std::shared_ptr<LuaData>> initializers;
  };
}

#endif