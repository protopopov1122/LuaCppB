#include "luacppb/Core/GC.h"

namespace LuaCppB::Internal {


  LuaGC::LuaGC() : state(nullptr) {}
  LuaGC::LuaGC(lua_State *state) : state(state) {}

  bool LuaGC::isValid() const {
    return this->state != nullptr;
  }

  bool LuaGC::isRunning() const {
    if (this->state) {
      return static_cast<bool>(lua_gc(this->state, LUA_GCISRUNNING, 0));
    } else {
      return false;
    }
  }

  int LuaGC::count() const {
    if (this->state) {
      return lua_gc(this->state, LUA_GCCOUNT, 0);
    } else {
      return 0;
    }
  }
  
  int LuaGC::countBytes() const {
    if (this->state) {
      return lua_gc(this->state, LUA_GCCOUNTB, 0);
    } else {
      return 0;
    }
  }

  void LuaGC::stop() {
    if (this->state) {
      lua_gc(this->state, LUA_GCSTOP, 0);
    }
  }

  void LuaGC::restart() {
    if (this->state) {
      lua_gc(this->state, LUA_GCRESTART, 0);
    }
  }

  void LuaGC::collect() {
    if (this->state) {
      lua_gc(this->state, LUA_GCCOLLECT, 0);
    }
  }

  bool LuaGC::step(int data) {
    if (this->state) {
      return static_cast<bool>(lua_gc(this->state, LUA_GCSTEP, data));
    } else {
      return false;
    }
  }

  int LuaGC::setPause(int data) {
    if (this->state) {
      return lua_gc(this->state, LUA_GCSETPAUSE, data);
    } else {
      return 0;
    }
  }

  int LuaGC::setStepMultiplier(int data) {
    if (this->state) {
      return lua_gc(this->state, LUA_GCSETSTEPMUL, data);
    } else {
      return 0;
    }
  }
}