#ifndef LUACPPB_CORE_STACKGUARD_H_
#define LUACPPB_CORE_STACKGUARD_H_

#include "luacppb/Base.h"
#include <cinttypes>

namespace LuaCppB::Internal {

  class LuaStackCanary {
   public:
    LuaStackCanary(lua_State *);

    bool check(int = 0) const noexcept;
    void assume(int = 0) const;
   private:
    lua_State *state;
    int size;
  };

  class LuaStackGuard {
   public:
    LuaStackGuard(lua_State *);

    std::size_t size() const noexcept;
    bool checkCapacity(std::size_t) const noexcept;
    void assumeCapacity(std::size_t) const;
    bool checkIndex(int) const noexcept;
    void assumeIndex(int) const;
    LuaStackCanary canary() const noexcept;
   private:
    lua_State *state;
  };

  class LuaStackCleaner {
   public:
    LuaStackCleaner(lua_State *);
    ~LuaStackCleaner();

    int getDelta();
   private:
    lua_State *state;
    int top;
  };
}

#endif