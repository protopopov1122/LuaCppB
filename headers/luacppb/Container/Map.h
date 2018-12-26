#ifndef LUACPPB_CONTAINER_MAP_H_
#define LUACPPB_CONTAINER_MAP_H_

#include "luacppb/Base.h"
#include "luacppb/Meta.h"
#include "luacppb/Core/Stack.h"

namespace LuaCppB::Internal {

  template <class P>
  class LuaCppMap {
   public:
    template <typename M>
    static typename std::enable_if<is_instantiation<std::map, M>::value>::type
      push(lua_State *state, LuaCppRuntime &runtime, M &map) {
      Internal::LuaStack stack(state);
      LuaCppObjectWrapper<M> *handle = stack.push<LuaCppObjectWrapper<M>>();
      new(handle) LuaCppObjectWrapper<M>(map);
      LuaCppMap<P>::set_map_meta<M>(state, runtime);
    }

    template <typename M>
    static typename std::enable_if<is_instantiation<std::map, M>::value>::type
      push(lua_State *state, LuaCppRuntime &runtime, const M &map) {
      Internal::LuaStack stack(state);
      LuaCppObjectWrapper<const M> *handle = stack.push<LuaCppObjectWrapper<const M>>();
      new(handle) LuaCppObjectWrapper<const M>(map);
      LuaCppMap<P>::set_map_meta<const M>(state, runtime);
    }

    template <typename M>
    static typename std::enable_if<is_instantiation<std::map, M>::value>::type
      push(lua_State *state, LuaCppRuntime &runtime, std::unique_ptr<M> &map) {
      Internal::LuaStack stack(state);
      LuaCppObjectWrapper<M> *handle = stack.push<LuaCppObjectWrapper<M>>();
      new(handle) LuaCppObjectWrapper<M>(std::move(map));
      LuaCppMap<P>::set_map_meta<M>(state, runtime);
    }

    template <typename M>
    static typename std::enable_if<is_instantiation<std::map, M>::value>::type
      push(lua_State *state, LuaCppRuntime &runtime, std::shared_ptr<M> &map) {
      Internal::LuaStack stack(state);
      LuaCppObjectWrapper<M> *handle = stack.push<LuaCppObjectWrapper<M>>();
      new(handle) LuaCppObjectWrapper<M>(map);
      LuaCppMap<P>::set_map_meta<M>(state, runtime);
    }
   private:
    template <typename M>
    static void set_map_meta(lua_State *state, LuaCppRuntime &runtime) {
      Internal::LuaStack stack(state);
      std::string typeName = typeid(M).name();
      if (stack.metatable(typeName)) {
        stack.push(&runtime);
        stack.push(&LuaCppMap<P>::map_get<M>, 1);
        stack.setField(-2, "__index");
        if constexpr (!std::is_const<M>::value) {
          stack.push(&LuaCppMap<P>::map_put<M>);
          stack.setField(-2, "__newindex");
        }
        stack.push(&LuaCppMap<P>::map_size<M>);
        stack.setField(-2, "__len");
        stack.push(&runtime);
        stack.push(&LuaCppMap<P>::map_pairs<M>, 1);
        stack.setField(-2, "__pairs");
        stack.push(&runtime);
        stack.push(&LuaCppMap<P>::map_ipairs<M>, 1);
        stack.setField(-2, "__ipairs");
        stack.push(&LuaCppMap<P>::map_gc<M>);
        stack.setField(-2, "__gc");
      }
      stack.setMetatable(-2);
    }

    template <typename M>
    static int map_get(lua_State *state) {
      using K = typename M::key_type;
      Internal::LuaStack stack(state);
      using Handle = LuaCppObjectWrapper<M>;
      Handle *handle = stack.toPointer<Handle *>(1);
      LuaCppRuntime &runtime = *stack.toPointer<LuaCppRuntime *>(lua_upvalueindex(1));
      K key = stack.get(2).value_or(LuaValue()).get<K>();
      M *map = handle->get();
      if (map && map->count(key) > 0) {
        P::push(state, runtime, map->at(key));
      } else {
        stack.push();
      }
      return 1;
    }

    template <typename M>
    static int map_put(lua_State *state) {
      using K = typename M::key_type;
      using V = typename M::mapped_type;
      Internal::LuaStack stack(state);
      using Handle = LuaCppObjectWrapper<M>;
      Handle *handle = stack.toPointer<Handle *>(1);
      K key = stack.get(2).value_or(LuaValue()).get<K>();
      V value = stack.get(3).value_or(LuaValue()).get<V>();
      M *map = handle->get();
      if (map) {
        (*map)[key] = value;
      }
      return 0;
    }

    template <typename M>
    static int map_size(lua_State *state) {
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

    template <typename M>
    static int map_gc(lua_State *state) {
      Internal::LuaStack stack(state);
      using Handle = LuaCppObjectWrapper<M>;
      Handle *handle = stack.toPointer<Handle *>(1);
      if (handle) {
        handle->~LuaCppObjectWrapper();
        ::operator delete(handle, handle);
      }
      return 0;
    }

    template <typename M>
    static int map_pairs(lua_State *state) {
      Internal::LuaStack stack(state);
      stack.copy(lua_upvalueindex(1));
      stack.push(&LuaCppMap<P>::map_iter<M>, 1);
      stack.copy(1);
      stack.push();
      return 3;
    }

    template <typename M>
    static int map_ipairs(lua_State *state) {
      Internal::LuaStack stack(state);
      stack.copy(lua_upvalueindex(1));
      stack.push(&LuaCppMap<P>::map_iiter<M>, 1);
      stack.copy(1);
      stack.push();
      return 3;
    }

    template <typename M>
    static int map_iter(lua_State *state) {
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
        K key = stack.get(2).value_or(LuaValue()).get<K>();
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

    template <typename M>
    static int map_iiter(lua_State *state) {
      using Handle = LuaCppObjectWrapper<M>;
      Internal::LuaStack stack(state);
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
        P::push(state, runtime, iter->first);
        P::push(state, runtime, iter->second);
        return 2;
      } else {
        return 0;
      }
    }
  };
}

#endif