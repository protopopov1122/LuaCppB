#ifndef LUACPPB_INVOKE_IMPL_DESCRIPTOR_H_
#define LUACPPB_INVOKE_IMPL_DESCRIPTOR_H_

#include "luacppb/Invoke/Descriptor.h"

namespace LuaCppB::Internal {

  template <typename C, typename M>
  NativeMethodDescriptor<C, M> *NativeMethodDescriptor<C, M>::pushDescriptor(lua_State *state) {
    Internal::LuaStack stack(state);
    NativeMethodDescriptor<C, M> *descriptor = stack.push<NativeMethodDescriptor<C, M>>();
    stack.pushTable();
    stack.push(&NativeMethodDescriptor<C, M>::gcDescriptor);
    stack.setField(-2, "__gc");
    stack.setMetatable(-2);
    return descriptor;
  }

  template <typename C, typename M>
  int NativeMethodDescriptor<C, M>::gcDescriptor(lua_State *state) {
    Internal::LuaStack stack(state);
    NativeMethodDescriptor<C, M> *descriptor = stack.toUserData<NativeMethodDescriptor<C, M> *>(1);
    if (descriptor) {
      descriptor->~NativeMethodDescriptor();
      ::operator delete(descriptor, descriptor);
    }
    return 0;
  }

  template <typename T>
	NativeInvocableDescriptor<T>::NativeInvocableDescriptor(T &value) : invocable(value) {}

  template <typename T>
  NativeInvocableDescriptor<T> *NativeInvocableDescriptor<T>::pushDescriptor(lua_State *state) {
    Internal::LuaStack stack(state);
    NativeInvocableDescriptor<T> *descriptor = stack.push<NativeInvocableDescriptor<T>>();
    stack.pushTable();
    stack.push(&NativeInvocableDescriptor<T>::gcDescriptor);
    stack.setField(-2, "__gc");
    stack.setMetatable(-2);
    return descriptor;
  }

  template <typename T>
  int NativeInvocableDescriptor<T>::gcDescriptor(lua_State *state) {
    Internal::LuaStack stack(state);
    NativeInvocableDescriptor<T> *descriptor = stack.toUserData<NativeInvocableDescriptor<T> *>(1);
    if (descriptor) {
      descriptor->~NativeInvocableDescriptor();
      ::operator delete(descriptor, descriptor);
    }
    return 0;
  }
}

#endif