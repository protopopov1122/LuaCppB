#ifndef LUACPPB_OBJECT_IMPL_FIELD_H_
#define LUACPPB_OBJECT_IMPL_FIELD_H_

#include "luacppb/Object/Field.h"

namespace LuaCppB::Internal {

  template <typename C, typename T>
  LuaCppObjectFieldHandle<C, T>::LuaCppObjectFieldHandle(T C::*field, LuaCppRuntime &runtime)
    : field(field), runtime(runtime) {}
  
  template <typename C, typename T>
  void LuaCppObjectFieldHandle<C, T>::push(lua_State *state, void *objPtr) const {
    C &object = *static_cast<LuaCppObjectWrapper<C> *>(objPtr)->get();
    Internal::LuaNativeValue::push(state, this->runtime, object.*(this->field));
  }
}

#endif