#ifndef LUACPPB_OBJECT_REGISTRY_H_
#define LUACPPB_OBJECT_REGISTRY_H_

#include "luacppb/Object/Class.h"
#include "luacppb/Object/Boxer.h"
#include "luacppb/Core/Error.h"
#include "luacppb/Core/Stack.h"
#include <typeindex>
#include <typeinfo>

namespace LuaCppB {

  template <typename T>
  class LuaCppClassObjectBoxer : public LuaCppObjectBoxer {
   public:
    LuaCppClassObjectBoxer(const std::string &className)
      : className(className) {}

    void wrap(lua_State *state, void *raw_ptr) override {
      LuaStack stack(state);
      T *object = reinterpret_cast<T *>(raw_ptr);
      LuaCppObjectWrapper<T> *wrapper = stack.push<LuaCppObjectWrapper<T>>();
      new(wrapper) LuaCppObjectWrapper<T>(object);
      stack.setMetatable(this->className);
    }

    void wrap(lua_State *state, const void *raw_ptr) override {
      LuaStack stack(state);
      const T *object = reinterpret_cast<const T *>(raw_ptr);
      LuaCppObjectWrapper<const T> *wrapper = stack.push<LuaCppObjectWrapper<const T>>();
      new(wrapper) LuaCppObjectWrapper<const T>(object);
      stack.setMetatable(this->className);
    }

    void wrapUnique(lua_State *state, void *raw_ptr) override {
      LuaStack stack(state);
      std::unique_ptr<T> object = std::unique_ptr<T>(reinterpret_cast<T *>(raw_ptr));
      LuaCppObjectWrapper<T> *wrapper = stack.push<LuaCppObjectWrapper<T>>();
      new(wrapper) LuaCppObjectWrapper<T>(std::move(object));
      stack.setMetatable(this->className);
    }

    void wrapShared(lua_State *state, std::shared_ptr<void> raw_object) override {
      LuaStack stack(state);
      std::shared_ptr<T> object = std::reinterpret_pointer_cast<T>(raw_object);
      LuaCppObjectWrapper<T> *wrapper = stack.push<LuaCppObjectWrapper<T>>();
      new(wrapper) LuaCppObjectWrapper<T>(object);
      stack.setMetatable(this->className);
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