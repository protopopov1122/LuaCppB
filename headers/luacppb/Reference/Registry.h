#ifndef LUACPPB_REFERENCE_REGISTRY_H_
#define LUACPPB_REFERENCE_REGISTRY_H_

#include "luacppb/Base.h"
#include <functional>

namespace LuaCppB {

  class LuaRegistryHandle {
   public:
    LuaRegistryHandle();
    LuaRegistryHandle(lua_State *, int = -1);
    LuaRegistryHandle(const LuaRegistryHandle &);
    virtual ~LuaRegistryHandle();

    void get(std::function<void(lua_State *)>) const;
    void set(std::function<void(lua_State *)>);
  //  private:
    lua_State *state;
    int ref;
  };
}

#endif