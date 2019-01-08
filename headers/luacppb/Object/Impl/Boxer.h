#ifndef LUACPPB_OBJECT_IMPL_BOXER_H_
#define LUACPPB_OBJECT_IMPL_BOXER_H_

#include "luacppb/Object/Boxer.h"

namespace LuaCppB::Internal {
    
  template <typename T>
  std::string LuaCppObjectBoxerRegistry::getClassName() {
    std::type_index idx = std::type_index(typeid(T));
    if (this->wrappers.count(idx)) {
      return this->wrappers[idx]->getClassName();
    } else {
      return "";
    }
  }

  template <typename T>
  bool LuaCppObjectBoxerRegistry::canWrap() {
    return this->wrappers.count(typeid(T)) != 0;
  }

  template <typename T>
  void LuaCppObjectBoxerRegistry::wrap(lua_State *state, T *object) {
    if (this->canWrap<T>()) {
      if constexpr (std::is_const<T>::value) {
        this->wrappers[typeid(T)]->wrap(state, reinterpret_cast<const void *>(object));
      } else {
        this->wrappers[typeid(T)]->wrap(state, reinterpret_cast<void *>(object));
      }
    }
  }

  template <typename T>
  void LuaCppObjectBoxerRegistry::wrap(lua_State *state, std::unique_ptr<T> object) {
    if (this->canWrap<T>()) {
      T *pointer = object.release();
      this->wrappers[typeid(T)]->wrapUnique(state, reinterpret_cast<void *>(pointer));
    }
  }

  template <typename T>
  void LuaCppObjectBoxerRegistry::wrap(lua_State *state, std::shared_ptr<T> object) {
    if (this->canWrap<T>()) {
      this->wrappers[typeid(T)]->wrapShared(state, std::static_pointer_cast<void >(object));
    }
  }

  template <typename T>
  void LuaCppObjectBoxerRegistry::addBoxer(std::shared_ptr<Internal::LuaCppObjectBoxer> wrapper) {
    this->wrappers[typeid(T)] = wrapper;
  }
}

#endif