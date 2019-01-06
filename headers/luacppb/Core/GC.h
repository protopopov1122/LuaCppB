#ifndef LUACPPB_CORE_GC_H_
#define LUACPPB_CORE_GC_H_

#include "luacppb/Base.h"

namespace LuaCppB::Internal {

  class LuaGC {
   public:
    LuaGC();
    LuaGC(lua_State *);

    bool isValid() const;
    bool isRunning() const;

    int count() const;
    int countBytes() const;

    void stop();
    void restart();
    void collect();

    bool step(int);
    int setPause(int);
    int setStepMultiplier(int);
   private:
    lua_State *state;
  };
}

#endif