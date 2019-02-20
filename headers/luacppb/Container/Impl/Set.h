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

#ifndef LUACPPB_CONTAINER_IMPL_SET_H_
#define LUACPPB_CONTAINER_IMPL_SET_H_

#include "luacppb/Container/Set.h"

namespace LuaCppB::Internal {

  template <class P>
  template <typename S>
  typename std::enable_if<is_instantiation<std::set, S>::value>::type
    LuaCppSet<P>::push(lua_State *state, LuaCppRuntime &runtime, S &set) {
    Internal::LuaStack stack(state);
    LuaCppObjectWrapper<S> *handle = stack.push<LuaCppObjectWrapper<S>>();
    new(handle) LuaCppObjectWrapper<S>(set);
    LuaCppSet<P>::set_set_meta<S>(state, runtime);
  }

  template <typename P>
  template <typename S>
  typename std::enable_if<is_instantiation<std::set, S>::value>::type
    LuaCppSet<P>::push(lua_State *state, LuaCppRuntime &runtime, const S &set) {
    Internal::LuaStack stack(state);
    LuaCppObjectWrapper<const S> *handle = stack.push<LuaCppObjectWrapper<const S>>();
    new(handle) LuaCppObjectWrapper<const S>(set);
    LuaCppSet<P>::set_set_meta<const S>(state, runtime);
  }

  template <typename P>
  template <typename S>
  typename std::enable_if<is_instantiation<std::set, S>::value>::type
    LuaCppSet<P>::push(lua_State *state, LuaCppRuntime &runtime, std::unique_ptr<S> &set) {
    Internal::LuaStack stack(state);
    LuaCppObjectWrapper<S> *handle = stack.push<LuaCppObjectWrapper<S>>();
    new(handle) LuaCppObjectWrapper<S>(std::move(set));
    LuaCppSet<P>::set_set_meta<S>(state, runtime);
  }

  template <typename P>
  template <typename S>
  typename std::enable_if<is_instantiation<std::set, S>::value>::type
    LuaCppSet<P>::push(lua_State *state, LuaCppRuntime &runtime, std::shared_ptr<S> &set) {
    Internal::LuaStack stack(state);
    LuaCppObjectWrapper<S> *handle = stack.push<LuaCppObjectWrapper<S>>();
    new(handle) LuaCppObjectWrapper<S>(set);
    LuaCppSet<P>::set_set_meta<S>(state, runtime);
  }

  template <class T1, typename T2>
  int LuaCppSet_put(lua_State *state) {
	return LuaCppSet<T1>::template set_put<T2>(state);
  }
  
  template <typename P>
  template <typename S>
  void LuaCppSet<P>::set_set_meta(lua_State *state, LuaCppRuntime &runtime) {
    Internal::LuaStack stack(state);
    std::string typeName = typeid(S).name();
    if (stack.metatable(typeName)) {
      stack.push(&LuaCppSet<P>::set_get<S>);
      stack.setField(-2, "__index");
      if constexpr (!std::is_const<S>::value) {
        stack.push(&LuaCppSet_put<P, S>);
        stack.setField(-2, "__newindex");
      }
      stack.push(&LuaCppSet<P>::set_size<S>);
      stack.setField(-2, "__len");
      stack.push(&runtime);
      stack.push(&LuaCppSet<P>::set_pairs<S>, 1);
      stack.setField(-2, "__pairs");
      stack.push(&LuaCppSet<P>::set_gc<S>);
      stack.setField(-2, "__gc");
      stack.pushTable();
      stack.setField(-2, "__metatable");
    }
    stack.setMetatable(-2);
  }

  template <typename P>
  template <typename S>
  int LuaCppSet<P>::set_get(lua_State *state) {
    using K = typename S::key_type;
    Internal::LuaStack stack(state);
    using Handle = LuaCppObjectWrapper<S>;
    Handle *handle = stack.toPointer<Handle *>(1);
    K key = stack.get(2).value_or(LuaValue::Nil).get<K>();
    S *set = handle->get();
    if (set && set->count(key) > 0) {
      stack.push(true);
    } else {
      stack.push();
    }
    return 1;
  }

  template <typename P>
  template <typename S>
  int LuaCppSet<P>::set_put(lua_State *state) {
    using K = typename S::key_type;
    Internal::LuaStack stack(state);
    using Handle = LuaCppObjectWrapper<S>;
    Handle *handle = stack.toPointer<Handle *>(1);
    K key = stack.get(2).value_or(LuaValue::Nil).get<K>();
    bool value = stack.is<LuaType::Boolean>(3) && stack.get(3).value().get<bool>();
    S *set = handle->get();
    if (set) {
      if (value) {
        set->insert(key);
      } else {
        set->erase(set->find(key));
      }
    }
    return 0;
  }

  template <typename P>
  template <typename S>
  int LuaCppSet<P>::set_size(lua_State *state) {
    Internal::LuaStack stack(state);
    using Handle = LuaCppObjectWrapper<S>;
    Handle *handle = stack.toPointer<Handle *>(1);
    S *set = handle->get();
    if (set) {
      stack.push(set->size());
    } else {
      stack.push();
    }
    return 1;
  }
  
  template <typename P>
  template <typename S>
  int LuaCppSet<P>::set_pairs(lua_State *state) {
    Internal::LuaStack stack(state);
    stack.copy(lua_upvalueindex(1));
    stack.push(&LuaCppSet<P>::set_iter<S>, 1);
    stack.copy(1);
    stack.push();
    return 3;
  }

  template <typename P>
  template <typename S>
  int LuaCppSet<P>::set_iter(lua_State *state) {
    using Handle = LuaCppObjectWrapper<S>;
    using K = typename S::key_type;
    Internal::LuaStack stack(state);
    LuaCppRuntime &runtime = *stack.toPointer<LuaCppRuntime *>(lua_upvalueindex(1));
    Handle *handle = stack.toPointer<Handle *>(1);
    S *set = handle->get();
    if (set == nullptr) {
      return 0;
    }
    typename S::const_iterator iter = set->begin();
    if (!stack.is<LuaType::Nil>(2)) {
      K key = stack.get(2).value_or(LuaValue::Nil).get<K>();
      iter = set->find(key);
      iter++;
    }
    if (iter != set->end()) {
      P::push(state, runtime, *iter);
      stack.push(true);
      return 2;
    } else {
      return 0;
    }
  }

  template <typename P>
  template <typename S>
  int LuaCppSet<P>::set_gc(lua_State *state) {
    Internal::LuaStack stack(state);
    using Handle = LuaCppObjectWrapper<S>;
    Handle *handle = stack.toPointer<Handle *>(1);
    if (handle) {
      handle->~LuaCppObjectWrapper();
      ::operator delete(handle, handle);
    }
    return 0;
  }
}

#endif
