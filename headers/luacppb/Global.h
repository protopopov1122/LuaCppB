#ifndef LUACPPB_GLOBAL_H_
#define LUACPPB_GLOBAL_H_

#include "luacppb/Base.h"
#include <functional>

namespace LuaCppB {

  class LuaReferenceHandle;

  class LuaGlobalScope {
   public:
    LuaGlobalScope(lua_State *);

    void get(const std::string &, std::function<void(lua_State *)>);
    void set(const std::string &, std::function<void(lua_State *)>);
    LuaReferenceHandle operator[](const std::string &);
   private:
    lua_State *state;
  };
}

#endif