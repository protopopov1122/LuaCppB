#include "luacppb/Core/Debug.h"

namespace LuaCppB {

  LuaDebugFrame::LuaDebugFrame(lua_State *state, int level) {
    lua_getstack(state, level, &this->debug);
  }

  int LuaDebugFrame::getEvent() const {
    return this->debug.event;
  }

  std::string LuaDebugFrame::getName() const {
    return std::string(this->debug.name != nullptr ? this->debug.name : "");
  }

  std::string LuaDebugFrame::getNameWhat() const {
    return std::string(this->debug.namewhat);
  }

  std::string LuaDebugFrame::getWhat() const {
    return std::string(this->debug.what);
  }

  std::string LuaDebugFrame::getSource() const {
    return std::string(this->debug.source);
  }

  std::string LuaDebugFrame::getShortSource() const {
    return std::string(this->debug.short_src);
  }

  int LuaDebugFrame::getCurrentLine() const {
    return this->debug.currentline;
  }

  int LuaDebugFrame::getLineDefined() const {
    return this->debug.linedefined;
  }

  int LuaDebugFrame::getLastLineDefined() const {
    return this->debug.lastlinedefined;
  }

  unsigned char LuaDebugFrame::getUpvalues() const {
    return this->debug.nups;
  }

  unsigned char LuaDebugFrame::getParameters() const {
    return this->debug.nparams;
  }

  bool LuaDebugFrame::isVarArg() const {
    return static_cast<bool>(this->debug.isvararg);
  }

  bool LuaDebugFrame::isTailCall() const {
    return static_cast<bool>(this->debug.istailcall);
  }
}