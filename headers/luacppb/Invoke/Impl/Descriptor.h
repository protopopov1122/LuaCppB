/*
  SPDX short identifier: MIT

  Copyright 2018-2019 Jevgēnijs Protopopovs

  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"),
  to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
  and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
  IN THE SOFTWARE.
*/

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
