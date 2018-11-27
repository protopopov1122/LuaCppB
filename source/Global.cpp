#include "luacppb/Global.h"
#include "luacppb/Reference.h"

namespace LuaCppB {

  LuaGlobalScope::LuaGlobalScope(lua_State *state) : state(state) {}

  void LuaGlobalScope::get(const std::string &key, std::function<void(lua_State *)> callback) {
    lua_getglobal(this->state, key.c_str());
    callback(this->state);
    lua_pop(this->state, 1);
  }

  void LuaGlobalScope::set(const std::string &key, std::function<void(lua_State *)> generator) {
    generator(this->state);
    lua_setglobal(this->state, key.c_str());
  }

  LuaReferenceHandle LuaGlobalScope::operator[](const std::string &key) {
    return LuaReferenceHandle(std::make_unique<LuaGlobalVariable>(*this, key));
  }
}