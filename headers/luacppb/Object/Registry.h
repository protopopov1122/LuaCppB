#ifndef LUACPPB_OBJECT_REGISTRY_H_
#define LUACPPB_OBJECT_REGISTRY_H_

#include "luacppb/Object/Class.h"
#include "luacppb/Object/Boxer.h"
#include "luacppb/Core/Error.h"
#include <typeindex>
#include <typeinfo>

namespace LuaCppB {

  template <typename T>
  class LuaCppClassObjectBoxer : public LuaCppObjectBoxer {
   public:
    LuaCppClassObjectBoxer(const std::string &className)
      : className(className) {}

    void wrap(lua_State *state, void *raw_ptr) override {
      T *object = reinterpret_cast<T *>(raw_ptr);
      LuaCppObjectWrapper<T> *wrapper = reinterpret_cast<LuaCppObjectWrapper<T> *>(lua_newuserdata(state, sizeof(LuaCppObjectWrapper<T>)));
      new(wrapper) LuaCppObjectWrapper<T>(object);
      luaL_setmetatable(state, this->className.c_str());
    }

    void wrap(lua_State *state, const void *raw_ptr) override {
      const T *object = reinterpret_cast<const T *>(raw_ptr);
      LuaCppObjectWrapper<const T> *wrapper = reinterpret_cast<LuaCppObjectWrapper<const T> *>(lua_newuserdata(state, sizeof(LuaCppObjectWrapper<const T>)));
      new(wrapper) LuaCppObjectWrapper<const T>(object);
      luaL_setmetatable(state, this->className.c_str());
    }

    void wrapUnique(lua_State *state, void *raw_ptr) override {
      std::unique_ptr<T> object = std::unique_ptr<T>(reinterpret_cast<T *>(raw_ptr));
      LuaCppObjectWrapper<T> *wrapper = reinterpret_cast<LuaCppObjectWrapper<T> *>(lua_newuserdata(state, sizeof(LuaCppObjectWrapper<T>)));
      new(wrapper) LuaCppObjectWrapper<T>(std::move(object));
      luaL_setmetatable(state, this->className.c_str());
    }

    void wrapShared(lua_State *state, std::shared_ptr<void> raw_object) override {
      std::shared_ptr<T> object = std::reinterpret_pointer_cast<T>(raw_object);
      LuaCppObjectWrapper<T> *wrapper = reinterpret_cast<LuaCppObjectWrapper<T> *>(lua_newuserdata(state, sizeof(LuaCppObjectWrapper<T>)));
      new(wrapper) LuaCppObjectWrapper<T>(object);
      luaL_setmetatable(state, this->className.c_str());
    }
   private:
    std::string className;
  };

  class LuaCppClassRegistry : public LuaCppObjectBoxerRegistry {
   public:
    using LuaCppObjectBoxerRegistry::LuaCppObjectBoxerRegistry;

    template <typename T>
    void bind(LuaCppClass<T> &cl) {
      cl.bind(this->state);
      this->addBoxer<T>(std::make_shared<LuaCppClassObjectBoxer<T>>(cl.getClassName()));
    }
  };
}

#endif