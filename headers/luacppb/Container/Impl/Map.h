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

#ifndef LUACPPB_CONTAINER_IMPL_MAP_H_
#define LUACPPB_CONTAINER_IMPL_MAP_H_

#include "luacppb/Container/Map.h"

namespace LuaCppB::Internal {
  template <typename P>
  template <typename M>
  typename std::enable_if<is_instantiation<std::map, M>::value>::type
    LuaCppMap<P>::push(lua_State *state, LuaCppRuntime &runtime, M &map) {
    Internal::LuaStack stack(state);
    LuaCppObjectWrapper<M> *handle = stack.push<LuaCppObjectWrapper<M>>();
    new(handle) LuaCppObjectWrapper<M>(map);
    LuaCppMap<P>::set_map_meta<M>(state, runtime);
  }

  template <typename P>
  template <typename M>
  typename std::enable_if<is_instantiation<std::map, M>::value>::type
    LuaCppMap<P>::push(lua_State *state, LuaCppRuntime &runtime, const M &map) {
    Internal::LuaStack stack(state);
    LuaCppObjectWrapper<const M> *handle = stack.push<LuaCppObjectWrapper<const M>>();
    new(handle) LuaCppObjectWrapper<const M>(map);
    LuaCppMap<P>::set_map_meta<const M>(state, runtime);
  }

  template <typename P>
  template <typename M, typename D>
  typename std::enable_if<is_instantiation<std::map, M>::value>::type
    LuaCppMap<P>::push(lua_State *state, LuaCppRuntime &runtime, std::unique_ptr<M, D> &map) {
    Internal::LuaStack stack(state);
    LuaCppObjectWrapper<M, D> *handle = stack.push<LuaCppObjectWrapper<M, D>>();
    new(handle) LuaCppObjectWrapper<M, D>(std::move(map));
    LuaCppMap<P>::set_map_meta<M, D>(state, runtime);
  }

  template <typename P>
  template <typename M>
  typename std::enable_if<is_instantiation<std::map, M>::value>::type
    LuaCppMap<P>::push(lua_State *state, LuaCppRuntime &runtime, std::shared_ptr<M> &map) {
    Internal::LuaStack stack(state);
    LuaCppObjectWrapper<M> *handle = stack.push<LuaCppObjectWrapper<M>>();
    new(handle) LuaCppObjectWrapper<M>(map);
    LuaCppMap<P>::set_map_meta<M>(state, runtime);
  }
  
  template <class T1, typename T2>
  int LuaCppMap_put(lua_State *state) {
	return LuaCppMap<T1>::template map_put<T2>(state);
  }
  
  template <typename P>
  template <typename M, typename D>
  void LuaCppMap<P>::set_map_meta(lua_State *state, LuaCppRuntime &runtime) {
    Internal::LuaStack stack(state);
    std::string typeName = typeid(M).name();
    if (stack.metatable(typeName)) {
      stack.push(&runtime);
      stack.push(&LuaCppMap<P>::template map_get<M>, 1);
      stack.setField(-2, "__index");
      if constexpr (!std::is_const<M>::value) {
        stack.push(&LuaCppMap_put<P, M>);
        stack.setField(-2, "__newindex");
      }
      stack.push(&LuaCppMap<P>::template map_size<M>);
      stack.setField(-2, "__len");
      stack.push(&runtime);
      stack.push(&LuaCppMap<P>::template map_pairs<M>, 1);
      stack.setField(-2, "__pairs");
      stack.push(&runtime);
      stack.push(&LuaCppMap<P>::template map_ipairs<M>, 1);
      stack.setField(-2, "__ipairs");
      stack.push(&LuaCppMap<P>::template map_gc<M, D>);
      stack.setField(-2, "__gc");
      stack.pushTable();
      stack.setField(-2, "__metatable");
    }
    stack.setMetatable(-2);
  }

  template <typename P>
  template <typename M>
  int LuaCppMap<P>::map_get(lua_State *state) {
    using K = typename M::key_type;
    Internal::LuaStack stack(state);
    using Handle = LuaCppObjectWrapper<M>;
    Handle *handle = stack.toPointer<Handle *>(1);
    LuaCppRuntime &runtime = *stack.toPointer<LuaCppRuntime *>(lua_upvalueindex(1));
    K key = stack.get(2).value_or(LuaValue::Nil).get<K>();
    M *map = handle->get();
    if (map && map->count(key) > 0) {
      P::push(state, runtime, map->at(key));
    } else {
      stack.push();
    }
    return 1;
  }

  template <typename P>
  template <typename M>
  int LuaCppMap<P>::map_put(lua_State *state) {
    using K = typename M::key_type;
    using V = typename M::mapped_type;
    Internal::LuaStack stack(state);
    using Handle = LuaCppObjectWrapper<M>;
    Handle *handle = stack.toPointer<Handle *>(1);
    K key = stack.get(2).value_or(LuaValue::Nil).get<K>();
    M *map = handle->get();
    if (!stack.is<LuaType::Nil>(3)) {
      V value = stack.get(3).value_or(LuaValue::Nil).get<V>();
      if (map) {
        (*map)[key] = value;
      }
    } else if (map) {
      map->erase(map->find(key));
    }
    return 0;
  }

  template <typename P>
  template <typename M>
  int LuaCppMap<P>::map_size(lua_State *state) {
    Internal::LuaStack stack(state);
    using Handle = LuaCppObjectWrapper<M>;
    Handle *handle = stack.toPointer<Handle *>(1);
    M *map = handle->get();
    if (map) {
      stack.push(map->size());
    } else {
      stack.push();
    }
    return 1;
  }

  template <typename P>
  template <typename M, typename D>
  int LuaCppMap<P>::map_gc(lua_State *state) {
    Internal::LuaStack stack(state);
    using Handle = LuaCppObjectWrapper<M, D>;
    Handle *handle = stack.toPointer<Handle *>(1);
    if (handle) {
      handle->~LuaCppObjectWrapper();
      ::operator delete(handle, handle);
    }
    return 0;
  }

  template <typename P>
  template <typename M>
  int LuaCppMap<P>::map_pairs(lua_State *state) {
    Internal::LuaStack stack(state);
    stack.copy(lua_upvalueindex(1));
    stack.push(&LuaCppMap<P>::map_iter<M>, 1);
    stack.copy(1);
    stack.push();
    return 3;
  }

  template <typename P>
  template <typename M>
  int LuaCppMap<P>::map_ipairs(lua_State *state) {
    lua_pushvalue(state, lua_upvalueindex(1));
    lua_pushcclosure(state, &LuaCppMap<P>::map_iiter<M>, 1);
    lua_pushvalue(state, 1);
    lua_pushinteger(state, 0);
    return 3;
  }

  template <typename P>
  template <typename M>
  int LuaCppMap<P>::map_iter(lua_State *state) {
    using Handle = LuaCppObjectWrapper<M>;
    using K = typename M::key_type;
    Internal::LuaStack stack(state);
    LuaCppRuntime &runtime = *stack.toPointer<LuaCppRuntime *>(lua_upvalueindex(1));
    Handle *handle = stack.toPointer<Handle *>(1);
    M *map = handle->get();
    if (map == nullptr) {
      return 0;
    }
    typename M::const_iterator iter = map->begin();
    if (!stack.is<LuaType::Nil>(2)) {
      K key = stack.get(2).value_or(LuaValue::Nil).get<K>();
      iter = map->find(key);
      iter++;
    }
    if (iter != map->end()) {
      P::push(state, runtime, iter->first);
      P::push(state, runtime, iter->second);
      return 2;
    } else {
      return 0;
    }
  }

  template <typename P>
  template <typename M>
  int LuaCppMap<P>::map_iiter(lua_State *state) {
    using Handle = LuaCppObjectWrapper<M>;
    LuaStack stack(state);
    LuaCppRuntime &runtime = *stack.toPointer<LuaCppRuntime *>(lua_upvalueindex(1));
    Handle *handle = stack.toPointer<Handle *>(1);
    M *map = handle->get();
    if (map == nullptr) {
      return 0;
    }
    std::size_t index = stack.toInteger(2);
    typename M::const_iterator iter = map->begin();
    while (index--) {
      ++iter;
    }
    if (iter != map->end()) {
      P::push(state, runtime, stack.toInteger(2) + 1);
      P::push(state, runtime, iter->second);
      return 2;
    } else {
      return 0;
    }
  }
}

#endif
