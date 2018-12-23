#ifndef LUACPPB_CONTAINER_MAP_H_
#define LUACPPB_CONTAINER_MAP_H_

#include "luacppb/Base.h"
#include "luacppb/Meta.h"
#include "luacppb/Core/Stack.h"

namespace LuaCppB {

  template <class P>
  class LuaCppMap {
   public:
    template <typename M>
    static typename std::enable_if<is_instantiation<std::map, M>::value>::type push(lua_State *state, LuaCppRuntime &runtime, M &map) {
      LuaCppObjectWrapper<M> *handle = reinterpret_cast<LuaCppObjectWrapper<M> *>(lua_newuserdata(state, sizeof(LuaCppObjectWrapper<M>)));
      new(handle) LuaCppObjectWrapper<M>(map);
      LuaCppMap<P>::set_map_meta<M>(state, runtime);
    }
   private:
    template <typename M>
    static void set_map_meta(lua_State *state, LuaCppRuntime &runtime) {
      std::string typeName = typeid(M).name();
      if (luaL_newmetatable(state, typeName.c_str())) {
        lua_pushlightuserdata(state, reinterpret_cast<void *>(&runtime));
        lua_pushcclosure(state, &LuaCppMap<P>::map_get<M>, 1);
        lua_setfield(state, -2, "__index");
        lua_pushcclosure(state, &LuaCppMap<P>::map_put<M>, 0);
        lua_setfield(state, -2, "__newindex");
        lua_pushcclosure(state, &LuaCppMap<P>::map_size<M>, 0);
        lua_setfield(state, -2, "__len");
        lua_pushcclosure(state, &LuaCppMap<P>::map_gc<M>, 0);
        lua_setfield(state, -2, "__gc");
      }
      lua_setmetatable(state, -2);
    }

    template <typename M>
    static int map_get(lua_State *state) {
      using K = typename M::key_type;
      LuaStack stack(state);
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
      LuaStack stack(state);
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
      LuaStack stack(state);
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
      LuaStack stack(state);
      using Handle = LuaCppObjectWrapper<M>;
      Handle *handle = stack.toPointer<Handle *>(1);
      if (handle) {
        handle->~LuaCppObjectWrapper();
        ::operator delete(handle, handle);
      }
      return 0;
    }
  };
}

#endif