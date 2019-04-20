#include "luacppb/Core/Alloc.h"

#ifdef LUACPPB_CUSTOM_ALLOCATOR_SUPPORT

namespace LuaCppB {

  static void *LuaAllocator_alloc(void *ud, void *ptr, std::size_t osize, std::size_t nsize) {
    LuaAllocator &alloc = *reinterpret_cast<LuaAllocator *>(ud);
    if (nsize == 0) {
      alloc.deallocate(ptr, osize);
      return nullptr;
    } else if (ptr != nullptr) {
      return alloc.reallocate(ptr, osize, nsize);
    } else {
      LuaType type;
      switch (osize) {
        case LUA_TSTRING:
        case LUA_TTABLE:
        case LUA_TFUNCTION:
        case LUA_TUSERDATA:
        case LUA_TTHREAD:
          type = static_cast<LuaType>(osize);
          break;
        default:
          type = LuaType::None;
          break;
      }
      return alloc.allocate(type, nsize);
    }
  }

  void LuaAllocator::bind(lua_State *state, LuaAllocator &alloc) {
    lua_setallocf(state, LuaAllocator_alloc, reinterpret_cast<void *>(&alloc));
  }
}

#endif