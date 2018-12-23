#ifndef LUACPPB_OBJECT_CLASS_H_
#define LUACPPB_OBJECT_CLASS_H_

#include "luacppb/Base.h"
#include "luacppb/Invoke/Native.h"
#include "luacppb/Object/Method.h"
#include "luacppb/Core/Stack.h"
#include <map>
#include <type_traits>
#include <typeinfo>

namespace LuaCppB {

  template <typename C, typename ... A>
  std::unique_ptr<C> LuaCppConstructor(A... args) {
    return std::make_unique<C>(args...);
  }

  template <typename C>
  class LuaCppClass : public LuaData {
   public:
    LuaCppClass(const std::string &className, LuaCppRuntime &runtime)
      : className(className), runtime(runtime) {}
    
    LuaCppClass(LuaCppRuntime &runtime)
      : className(typeid(C).name()), runtime(runtime) {}

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
        for (auto it = this->staticMethods.begin(); it != this->staticMethods.end(); ++it) {
          it->second->push(state);
          lua_setfield(state, -2, it->first.c_str());
        }
        lua_pushstring(state, this->className.c_str());
        lua_pushcclosure(state, &LuaCppClass<C>::gcObject, 1);
        lua_setfield(state, -2, "__gc");
      }
    }

    void bind(lua_State *state) {
      this->push(state);
      lua_pop(state, 1);
    }

    template <typename R, typename ... A>
    void bind(const std::string &key, R (C::*method)(A...)) {
      using M = R (C::*)(A...);
      this->methods[key] = std::make_shared<LuaCppObjectMethodCall<C, M, R, A...>>(method, this->className, this->runtime);
    }

    template <typename R, typename ... A>
    void bind(const std::string &key, R (C::*method)(A...) const) {
      using M = R (C::*)(A...) const;
      this->methods[key] = std::make_shared<LuaCppObjectMethodCall<const C, M, R, A...>>(method, this->className, this->runtime);
    }

    template <typename R, typename ... A>
    void bind(const std::string &key, R (*function)(A...)) {
      this->staticMethods[key] = std::make_shared<NativeFunctionCall<R, A...>>(function, this->runtime);
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
    std::map<std::string, std::shared_ptr<LuaData>> staticMethods;
    LuaCppRuntime &runtime;
  };
}

#endif