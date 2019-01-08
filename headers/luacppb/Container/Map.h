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
      push(lua_State *, LuaCppRuntime &, M &);

    template <typename M>
    static typename std::enable_if<is_instantiation<std::map, M>::value>::type
      push(lua_State *, LuaCppRuntime &, const M &);

    template <typename M>
    static typename std::enable_if<is_instantiation<std::map, M>::value>::type
      push(lua_State *, LuaCppRuntime &, std::unique_ptr<M> &);

    template <typename M>
    static typename std::enable_if<is_instantiation<std::map, M>::value>::type
      push(lua_State *, LuaCppRuntime &, std::shared_ptr<M> &);
    
   private:
    template <typename M>
    static void set_map_meta(lua_State *, LuaCppRuntime &);

    template <typename M>
    static int map_get(lua_State *);

    template <typename M>
    static int map_put(lua_State *);

    template <typename M>
    static int map_size(lua_State *);

    template <typename M>
    static int map_gc(lua_State *);

    template <typename M>
    static int map_pairs(lua_State *);

    template <typename M>
    static int map_iter(lua_State *);
  };
}

#include "luacppb/Container/Impl/Map.h"

#endif