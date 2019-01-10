#ifndef LUACPPB_CONTAINER_SET_H_
#define LUACPPB_CONTAINER_SET_H_

#include "luacppb/Base.h"
#include "luacppb/Meta.h"
#include "luacppb/Core/Stack.h"
#include <set>

namespace LuaCppB::Internal {

  template <class P>
  class LuaCppSet {
   public:
    template <typename S>
    static typename std::enable_if<is_instantiation<std::set, S>::value>::type
      push(lua_State *, LuaCppRuntime &, S &);
    
    template <typename S>
    static typename std::enable_if<is_instantiation<std::set, S>::value>::type
      push(lua_State *, LuaCppRuntime &, const S &);

    template <typename S>
    static typename std::enable_if<is_instantiation<std::set, S>::value>::type
      push(lua_State *, LuaCppRuntime &, std::unique_ptr<S> &);

    template <typename S>
    static typename std::enable_if<is_instantiation<std::set, S>::value>::type
      push(lua_State *, LuaCppRuntime &, std::shared_ptr<S> &);
   private:
    template <typename S>
    static void set_set_meta(lua_State *, LuaCppRuntime &);

    template <typename S>
    static int set_get(lua_State *);

    template <typename S>
    static int set_put(lua_State *);

    template <typename S>
    static int set_size(lua_State *);

    template <typename S>
    static int set_pairs(lua_State *);

    template <typename S>
    static int set_iter(lua_State *);

    template <typename S>
    static int set_gc(lua_State *);
  };
}

#include "luacppb/Container/Impl/Set.h"

#endif