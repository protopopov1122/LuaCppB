#ifndef LUACPPB_OBJECT_REGISTRY_H_
#define LUACPPB_OBJECT_REGISTRY_H_

#include "luacppb/Object/Class.h"
#include "luacppb/Object/Boxer.h"
#include "luacppb/Core/Error.h"
#include "luacppb/Core/Stack.h"
#include <typeindex>
#include <typeinfo>

namespace LuaCppB::Internal {

  template <typename T, typename P>
  class LuaCppClassObjectBoxer : public LuaCppObjectBoxer {
   public:
    LuaCppClassObjectBoxer(const std::string &className)
      : className(className) {}

    void wrap(lua_State *state, void *raw_ptr) override {
      Internal::LuaStack stack(state);
      T *object = reinterpret_cast<T *>(raw_ptr);
      LuaCppObjectWrapper<T> *wrapper = stack.push<LuaCppObjectWrapper<T>>();
      new(wrapper) LuaCppObjectWrapper<T>(object);
      if constexpr (!std::is_void<P>::value) {
        wrapper->addParentType(std::type_index(typeid(P)));
      }
      stack.setMetatable(this->className);
    }

    void wrap(lua_State *state, const void *raw_ptr) override {
      Internal::LuaStack stack(state);
      const T *object = reinterpret_cast<const T *>(raw_ptr);
      LuaCppObjectWrapper<const T> *wrapper = stack.push<LuaCppObjectWrapper<const T>>();
      new(wrapper) LuaCppObjectWrapper<const T>(object);
      if constexpr (!std::is_void<P>::value) {
        wrapper->addParentType(std::type_index(typeid(P)));
      }
      stack.setMetatable(this->className);
    }

    void wrapUnique(lua_State *state, void *raw_ptr) override {
      Internal::LuaStack stack(state);
      std::unique_ptr<T> object = std::unique_ptr<T>(reinterpret_cast<T *>(raw_ptr));
      LuaCppObjectWrapper<T> *wrapper = stack.push<LuaCppObjectWrapper<T>>();
      new(wrapper) LuaCppObjectWrapper<T>(std::move(object));
      if constexpr (!std::is_void<P>::value) {
        wrapper->addParentType(std::type_index(typeid(P)));
      }
      stack.setMetatable(this->className);
    }

    void wrapShared(lua_State *state, std::shared_ptr<void> raw_object) override {
      Internal::LuaStack stack(state);
      std::shared_ptr<T> object = std::static_pointer_cast<T>(raw_object);
      LuaCppObjectWrapper<T> *wrapper = stack.push<LuaCppObjectWrapper<T>>();
      new(wrapper) LuaCppObjectWrapper<T>(object);
      if constexpr (!std::is_void<P>::value) {
        wrapper->addParentType(std::type_index(typeid(P)));
      }
      stack.setMetatable(this->className);
    }

    std::string getClassName() override {
      return this->className;
    }
   private:
    std::string className;
  };

  class LuaCppClassRegistry : public LuaCppObjectBoxerRegistry {
   public:
    using LuaCppObjectBoxerRegistry::LuaCppObjectBoxerRegistry;

    template <typename T, typename P>
    void bind(LuaCppClass<T, P> &cl) {
      cl.bind(this->state);
      this->addBoxer<T>(std::make_shared<LuaCppClassObjectBoxer<T, P>>(cl.getClassName()));
    }
  };
}

#endif
