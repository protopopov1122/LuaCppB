#ifndef LUACPPB_CONTAINER_IMPL_VECTOR_H_
#define LUACPPB_CONTAINER_IMPL_VECTOR_H_

#include "luacppb/Container/Vector.h"

namespace LuaCppB::Internal {

  template <typename T, typename A, typename E>
  int cpp_vector_newindex<T, A, E>::newindex(lua_State *state) {
    return 0;
  }

  template <typename T, typename A>
  int cpp_vector_newindex<T, A, typename std::enable_if<std::is_same<decltype(std::declval<LuaValue>().get<T>()), T>::value>::type>::newindex(lua_State *state) {
    using V = std::vector<T, A>;
    Internal::LuaStack stack(state);
    using Handle = LuaCppObjectWrapper<V>;
    Handle *handle = stack.toPointer<Handle *>(1);
    std::size_t index = stack.toInteger(2) - 1;
    T value = stack.get(3).value_or(LuaValue()).get<T>();
    V *vec = handle->get();
    if (vec) {
      if (index < vec->size()) {
        (*vec)[index] = value;
      } else if (index == vec->size()) {
        vec->push_back(value);
      }
    }
    return 0;
  }

  template <class P>
  template <typename V>
  typename std::enable_if<is_instantiation<std::vector, V>::value>::type
    LuaCppVector<P>::push(lua_State *state, LuaCppRuntime &runtime, V &vec) {
    Internal::LuaStack stack(state);
    LuaCppObjectWrapper<V> *handle = stack.push<LuaCppObjectWrapper<V>>();
    new(handle) LuaCppObjectWrapper<V>(vec);
    LuaCppVector<P>::set_vector_meta<V>(state, runtime);
  }

  template <class P>
  template <typename V>
  typename std::enable_if<is_instantiation<std::vector, V>::value>::type
    LuaCppVector<P>::push(lua_State *state, LuaCppRuntime &runtime, const V &vec) {
    Internal::LuaStack stack(state);
    LuaCppObjectWrapper<const V> *handle = stack.push<LuaCppObjectWrapper<const V>>();
    new(handle) LuaCppObjectWrapper<const V>(vec);
    LuaCppVector<P>::set_vector_meta<const V>(state, runtime);
  }

  template <class P>
  template <typename V>
  typename std::enable_if<is_instantiation<std::vector, V>::value>::type
    LuaCppVector<P>::push(lua_State *state, LuaCppRuntime &runtime, std::unique_ptr<V> &vec) {
    Internal::LuaStack stack(state);
    LuaCppObjectWrapper<V> *handle = stack.push<LuaCppObjectWrapper<V>>();
    new(handle) LuaCppObjectWrapper<V>(std::move(vec));
    LuaCppVector<P>::set_vector_meta<V>(state, runtime);
  }

  template <class P>
  template <typename V>
  typename std::enable_if<is_instantiation<std::vector, V>::value>::type
    LuaCppVector<P>::push(lua_State *state, LuaCppRuntime &runtime, std::shared_ptr<V> &vec) {
    Internal::LuaStack stack(state);
    LuaCppObjectWrapper<V> *handle = stack.push<LuaCppObjectWrapper<V>>();
    new(handle) LuaCppObjectWrapper<V>(vec);
    LuaCppVector<P>::set_vector_meta<V>(state, runtime);
  }
  
  template <class P>
  template <typename V>
  void LuaCppVector<P>::set_vector_meta(lua_State *state, LuaCppRuntime &runtime) {
    using T = typename V::value_type;
    using A = typename V::allocator_type;
    std::string typeName = typeid(V).name();
    if constexpr (std::is_const<V>::value) {
      typeName = typeName + "__const";
    }
    Internal::LuaStack stack(state);
    if (stack.metatable(typeName)) {
      stack.push(&runtime);
      stack.push(&LuaCppVector<P>::vector_index<V>, 1);
      stack.setField(-2, "__index");
      if constexpr (!std::is_const<V>::value) {
        stack.push(&cpp_vector_newindex<T, A>::newindex);
        stack.setField(-2, "__newindex");
      }
      stack.push(&runtime);
      stack.push(&LuaCppVector<P>::vector_pairs<V>, 1);
      stack.setField(-2, "__pairs");
      stack.push(&LuaCppVector<P>::vector_length<V>);
      stack.setField(-2, "__len");
      stack.push(&LuaCppVector::vector_gc<V>);
      stack.setField(-2, "__gc");
    }
    stack.setMetatable(-2);
  }

  template <class P>
  template <typename V>
  int LuaCppVector<P>::vector_index(lua_State *state) {
    Internal::LuaStack stack(state);
    using Handle = LuaCppObjectWrapper<V>;
    Handle *handle = stack.toPointer<Handle *>(1);
    LuaCppRuntime &runtime = *stack.toPointer<LuaCppRuntime *>(lua_upvalueindex(1));
    std::size_t index = stack.toInteger(2) - 1;
    V &vec = *handle->get();
    if (index >= vec.size()) {
      return 0;
    } else {
      P::push(state, runtime, vec.at(index));
      return 1;
    }
  }

  template <class P>
  template <typename V>
  int LuaCppVector<P>::vector_length(lua_State *state) {
    Internal::LuaStack stack(state);
    using Handle = LuaCppObjectWrapper<V>;
    Handle *handle = stack.toPointer<Handle *>(1);
    V &vec = *handle->get();
    stack.push(vec.size());
    return 1;
  }

  template <class P>
  template <typename V>
  int LuaCppVector<P>::vector_gc(lua_State *state) {
    Internal::LuaStack stack(state);
    using Handle = LuaCppObjectWrapper<V>;
    Handle *handle = stack.toPointer<Handle *>(1);
    if (handle) {
      handle->~LuaCppObjectWrapper();
      ::operator delete(handle, handle);
    }
    return 0;
  }

  template <class P>
  template <typename V>
  int LuaCppVector<P>::vector_pairs(lua_State *state) {
    Internal::LuaStack stack(state);
    stack.copy(lua_upvalueindex(1));
    stack.push(&LuaCppVector<P>::vector_iter<V>, 1);
    stack.copy(1);
    stack.push();
    return 3;
  }

  template <class P>
  template <typename V>
  int LuaCppVector<P>::vector_iter(lua_State *state) {
    using Handle = LuaCppObjectWrapper<V>;
    Internal::LuaStack stack(state);
    LuaCppRuntime &runtime = *stack.toPointer<LuaCppRuntime *>(lua_upvalueindex(1));
    Handle *handle = stack.toPointer<Handle *>(1);
    V *vec = handle->get();
    if (vec == nullptr) {
      return 0;
    }
    std::size_t index = 0;
    typename V::const_iterator iter = vec->begin();
    if (!stack.is<LuaType::Nil>(2)) {
      index = stack.toInteger(2);
      std::size_t key = index;
      while (key--) {
        ++iter;
      }
    }
    if (iter != vec->end()) {
      P::push(state, runtime, index + 1);
      P::push(state, runtime, *iter);
      return 2;
    } else {
      return 0;
    }
  }
}

#endif