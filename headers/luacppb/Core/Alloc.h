#ifndef LUACPPB_CORE_ALLOC_H_
#define LUACPPB_CORE_ALLOC_H_

#include "luacppb/Base.h"
#include "luacppb/Value/Types.h"

#ifdef LUACPPB_CUSTOM_ALLOCATOR_SUPPORT

namespace LuaCppB {

  class LuaAllocator {
   public:
    virtual ~LuaAllocator() = default;
    virtual void *allocate(LuaType, std::size_t) = 0;
    virtual void *reallocate(void *, std::size_t, std::size_t) = 0;
    virtual void deallocate(void *, std::size_t) = 0;

    static void bind(lua_State *, LuaAllocator &);
    static lua_State *create(LuaAllocator &);
  };
}

#endif

#endif