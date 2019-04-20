#ifndef LUACPPB_CORE_PANIC_H_
#define LUACPPB_CORE_PANIC_H_

#include "luacppb/Base.h"
#include <functional>
#include <map>

namespace LuaCppB::Internal {

  class LuaPanicDispatcher {
   public:
    using Handler = std::function<int(lua_State *)>;
    lua_CFunction attach(lua_State *, Handler);
    void detach(lua_State *);
    static LuaPanicDispatcher &getGlobal();
   private:
    static int atpanic(lua_State *);
    static LuaPanicDispatcher singleton;
    std::map<lua_State *, Handler> handlers;
  };

  class LuaPanicUnbind {
   public:
    LuaPanicUnbind(LuaPanicDispatcher &);
    LuaPanicUnbind(LuaPanicDispatcher &, lua_State *);
    LuaPanicUnbind(const LuaPanicUnbind &) = delete;
    LuaPanicUnbind(LuaPanicUnbind &&);
    LuaPanicUnbind &operator=(const LuaPanicUnbind &) = delete;
    LuaPanicUnbind &operator=(LuaPanicUnbind &&);
    ~LuaPanicUnbind();
   private:
    LuaPanicDispatcher &dispatcher;
    lua_State *state;
  };
}

#endif