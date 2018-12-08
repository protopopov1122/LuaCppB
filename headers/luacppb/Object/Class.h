#ifndef LUACPPB_OBJECT_CLASS_H_
#define LUACPPB_OBJECT_CLASS_H_

#include "luacppb/Base.h"
#include "luacppb/Object/Method.h"
#include <map>
#include <iostream>

namespace LuaCppB {

  template <typename C>
  class LuaCppClass : public LuaData {
   public:
    LuaCppClass(const std::string &className)
      : className(className) {}
    
    void push(lua_State *state) const override {
      luaL_newmetatable(state, this->className.c_str());
      lua_newtable(state);
      for (auto it = this->methods.begin(); it != this->methods.end(); ++it) {
        it->second->push(state);
        lua_setfield(state, -2, it->first.c_str());
      }
      lua_setfield(state, -2, "__index");
      lua_pushstring(state, this->className.c_str());
      lua_pushcclosure(state, &LuaCppClass<C>::newObject, 1);
      lua_setfield(state, -2, "new");
      lua_pushcfunction(state, &LuaCppClass<C>::gcObject);
      lua_setfield(state, -2, "__gc");
    }

    template <typename R, typename ... A>
    void bind(const std::string &key, R (C::*method)(A...)) {
      this->methods[key] = std::make_shared<CppObjectMethodCall<C, R, A...>>(NativeMethodWrapper(method).get());
    }

    template <typename R, typename ... A>
    void bind(const std::string &key, R (C::*method)(A...) const) {
      this->methods[key] = std::make_shared<CppObjectMethodCall<C, R, A...>>(NativeMethodWrapper(method).get());
    }
   private:
    static int newObject(lua_State *state) {
      const char *className = lua_tostring(state, lua_upvalueindex(1));
      C *object = reinterpret_cast<C *>(lua_newuserdata(state, sizeof(C)));
      new(object) C();
      luaL_setmetatable(state, className);
      return 1;
    }

    static int gcObject(lua_State *state) {
      C *object = reinterpret_cast<C *>(lua_touserdata(state, 1));
      object->~C();
      ::operator delete(object, object);
      return 0;
    }

    std::string className;
    std::map<std::string, std::shared_ptr<LuaData>> methods;
  };
}

#endif