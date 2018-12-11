#ifndef LUACPPB_OBJECT_REGISTRY_H_
#define LUACPPB_OBJECT_REGISTRY_H_

#include "luacppb/Object/Class.h"
#include <typeindex>
#include <typeinfo>
#include <iostream>

namespace LuaCppB {

  class LuaCppClassRegistry {
   public:
    LuaCppClassRegistry(lua_State *state) : state(state) {}
    template <typename T>
    void bind(LuaCppClass<T> &cl) {
      cl.push(this->state);
      lua_pop(this->state, 1);
      this->registry[typeid(T)] = cl.getClassName();
    }

    template <typename T>
    bool canWrap() {
      return this->registry.count(typeid(T)) != 0;
    }

    template <typename T>
    void wrap(lua_State *state, T *object) {
      LuaCppObjectWrapper<T> *wrapper = reinterpret_cast<LuaCppObjectWrapper<T> *>(lua_newuserdata(state, sizeof(LuaCppObjectWrapper<T>)));
      new(wrapper) LuaCppObjectWrapper<T>(object);
      luaL_setmetatable(state, this->registry[typeid(T)].c_str());
    }
   private:
    lua_State *state;
    std::map<std::type_index, std::string> registry;
  };
}

#endif