#ifndef LUACPPB_OBJECT_IMPL_REGISTRY_H_
#define LUACPPB_OBJECT_IMPL_REGISTRY_H_

#include "luacppb/Object/Registry.h"


namespace LuaCppB::Internal {

  template <typename T, typename P>
  LuaCppClassObjectBoxer<T, P>::LuaCppClassObjectBoxer(const std::string &className, std::map<std::string, std::shared_ptr<LuaCppObjectFieldPusher>> &fields)
    : className(className), fields(fields) {}

  template <typename T, typename P>
  void LuaCppClassObjectBoxer<T, P>::wrap(lua_State *state, void *raw_ptr) {
    Internal::LuaStack stack(state);
    T *object = reinterpret_cast<T *>(raw_ptr);
    LuaCppObjectWrapper<T> *wrapper = stack.push<LuaCppObjectWrapper<T>>();
    new(wrapper) LuaCppObjectWrapper<T>(object);
    if constexpr (!std::is_void<P>::value) {
      wrapper->addParentType(std::type_index(typeid(P)));
    }
    stack.setMetatable(this->className);
  }

  template <typename T, typename P>
  void LuaCppClassObjectBoxer<T, P>::wrap(lua_State *state, const void *raw_ptr) {
    Internal::LuaStack stack(state);
    const T *object = reinterpret_cast<const T *>(raw_ptr);
    LuaCppObjectWrapper<const T> *wrapper = stack.push<LuaCppObjectWrapper<const T>>();
    new(wrapper) LuaCppObjectWrapper<const T>(object);
    if constexpr (!std::is_void<P>::value) {
      wrapper->addParentType(std::type_index(typeid(P)));
    }
    stack.setMetatable(this->className);
  }

  template <typename T, typename P>
  void LuaCppClassObjectBoxer<T, P>::wrapUnique(lua_State *state, void *raw_ptr) {
    Internal::LuaStack stack(state);
    std::unique_ptr<T> object = std::unique_ptr<T>(reinterpret_cast<T *>(raw_ptr));
    LuaCppObjectWrapper<T> *wrapper = stack.push<LuaCppObjectWrapper<T>>();
    new(wrapper) LuaCppObjectWrapper<T>(std::move(object));
    if constexpr (!std::is_void<P>::value) {
      wrapper->addParentType(std::type_index(typeid(P)));
    }
    stack.setMetatable(this->className);
  }

  template <typename T, typename P>
  void LuaCppClassObjectBoxer<T, P>::wrapShared(lua_State *state, std::shared_ptr<void> raw_object) {
    Internal::LuaStack stack(state);
    std::shared_ptr<T> object = std::static_pointer_cast<T>(raw_object);
    LuaCppObjectWrapper<T> *wrapper = stack.push<LuaCppObjectWrapper<T>>();
    new(wrapper) LuaCppObjectWrapper<T>(object);
    if constexpr (!std::is_void<P>::value) {
      wrapper->addParentType(std::type_index(typeid(P)));
    }
    stack.setMetatable(this->className);
  }

  template <typename T, typename P>
  std::string LuaCppClassObjectBoxer<T, P>::getClassName() {
    return this->className;
  }

  template <typename T, typename P>
  void LuaCppClassObjectBoxer<T, P>::fillFields(std::map<std::string, std::shared_ptr<LuaCppObjectFieldPusher>> &fields) {
    fields.insert(this->fields.begin(), this->fields.end());
  }

  template <typename T, typename P>
  void LuaCppClassRegistry::bind(LuaCppClass<T, P> &cl) {
    cl.bind(this->state);
    std::map<std::string, std::shared_ptr<LuaCppObjectFieldPusher>> map;
    cl.fillFields(map);
    this->addBoxer<T>(std::make_shared<LuaCppClassObjectBoxer<T, P>>(cl.getClassName(), map));
  }
}

#endif