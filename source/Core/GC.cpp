/*
  SPDX short identifier: MIT

  Copyright 2018-2019 Jevgēnijs Protopopovs

  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"),
  to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
  and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
  IN THE SOFTWARE.
*/

#include "luacppb/Core/GC.h"

namespace LuaCppB::Internal {


  LuaGC::LuaGC() : state(nullptr) {}
  LuaGC::LuaGC(lua_State *state) : state(state) {}

  bool LuaGC::valid() const {
    return this->state != nullptr;
  }

  bool LuaGC::isRunning() const {
    if (this->state) {
      #ifdef LUACPPB_GC_ISRUNNING
      return static_cast<bool>(lua_gc(this->state, LUA_GCISRUNNING, 0));
      #else
      return false;
      #endif
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
