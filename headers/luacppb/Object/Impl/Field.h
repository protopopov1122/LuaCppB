#ifndef LUACPPB_OBJECT_IMPL_FIELD_H_
#define LUACPPB_OBJECT_IMPL_FIELD_H_

#include "luacppb/Object/Field.h"

namespace LuaCppB::Internal {

  template <typename C, typename B, typename T>
  LuaCppObjectFieldHandle<C, B, T>::LuaCppObjectFieldHandle(T C::*field, LuaCppRuntime &runtime)
    : field(field), runtime(runtime) {}
  
  template <typename C, typename B, typename T>
  void LuaCppObjectFieldHandle<C, B, T>::push(lua_State *state, void *objPtr) const {
    B &object = *static_cast<LuaCppObjectWrapper<B> *>(objPtr)->get();
    Internal::LuaNativeValue::push(state, this->runtime, object.*(this->field));
  }
}

#endif