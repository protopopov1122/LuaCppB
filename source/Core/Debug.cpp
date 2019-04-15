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

#include "luacppb/Core/Debug.h"

namespace LuaCppB {

  LuaDebugBaseFrame::LuaDebugBaseFrame(lua_State *state, LuaCppRuntime &runtime, lua_Debug *debug)
    : state(state), runtime(runtime), debug(*debug) {}

  LuaDebugBaseFrame::LuaDebugBaseFrame(lua_State *state, LuaCppRuntime &runtime, int level)
    : state(state), runtime(runtime) {
    lua_getstack(state, level, &this->debug);
  }

  int LuaDebugBaseFrame::getEvent() const {
    return this->debug.event;
  }

  std::string LuaDebugBaseFrame::getName() const {
    return std::string(this->debug.name != nullptr ? this->debug.name : "");
  }

  std::string LuaDebugBaseFrame::getNameWhat() const {
    return std::string(this->debug.namewhat);
  }

  std::string LuaDebugBaseFrame::getWhat() const {
    return std::string(this->debug.what);
  }

  std::string LuaDebugBaseFrame::getSource() const {
    return std::string(this->debug.source);
  }

  std::string LuaDebugBaseFrame::getShortSource() const {
    return std::string(this->debug.short_src);
  }

  int LuaDebugBaseFrame::getCurrentLine() const {
    return this->debug.currentline;
  }

  int LuaDebugBaseFrame::getLineDefined() const {
    return this->debug.linedefined;
  }

  int LuaDebugBaseFrame::getLastLineDefined() const {
    return this->debug.lastlinedefined;
  }

  unsigned char LuaDebugBaseFrame::getUpvalues() const {
    return this->debug.nups;
  }

  unsigned char LuaDebugBaseFrame::getParameters() const {
    return this->debug.nparams;
  }

  bool LuaDebugBaseFrame::isVarArg() const {
    return static_cast<bool>(this->debug.isvararg);
  }

  bool LuaDebugBaseFrame::isTailCall() const {
    return static_cast<bool>(this->debug.istailcall);
  }
}