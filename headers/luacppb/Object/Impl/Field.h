#ifndef LUACPPB_OBJECT_IMPL_FIELD_H_
#define LUACPPB_OBJECT_IMPL_FIELD_H_

#include "luacppb/Object/Field.h"

namespace LuaCppB::Internal {

  template <typename C, typename T>
  LuaCppObjectFieldHandle<C, T>::LuaCppObjectFieldHandle(C &object, T C::*field, LuaCppRuntime &runtime)
    : object(object), field(field), runtime(runtime) {}
  
  template <typename C, typename T>
  void LuaCppObjectFieldHandle<C, T>::push(lua_State *state) const {
    Internal::LuaNativeValue::push(state, this->runtime, this->object.*(this->field));
  }

  template <typename C, typename T>
  LuaCppObjectFieldHandleBuilder_Impl<C, T>::LuaCppObjectFieldHandleBuilder_Impl(T C::*field)
    : field(field) {}
  
  template <typename C, typename T>
  std::shared_ptr<LuaData> LuaCppObjectFieldHandleBuilder_Impl<C, T>::build(C &object, LuaCppRuntime &runtime) {
    return std::make_shared<LuaCppObjectFieldHandle<C, T>>(object, this->field, runtime);
  }
}

#endif