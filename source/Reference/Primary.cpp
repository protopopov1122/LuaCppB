#include "luacppb/Reference/Reference.h"

namespace LuaCppB {

  void LuaGlobalVariable::putOnTop(std::function<void (lua_State *)> callback) {
    lua_getglobal(this->state, this->name.c_str());
    callback(this->state);
    lua_pop(this->state, 1);
  }

  void LuaGlobalVariable::setValue(std::function<void (lua_State *)> gen) {
    gen(this->state);
    lua_setglobal(this->state, this->name.c_str());
  }

  void LuaStackReference::putOnTop(std::function<void (lua_State *)> callback) {
    lua_pushvalue(this->state, this->index);
    callback(this->state);
    lua_pop(this->state, 1);
  }

  void LuaStackReference::setValue(std::function<void (lua_State *)> gen) {
  }

  LuaRegistryReference::LuaRegistryReference(lua_State *state, int index)
    : handle(state, index) {
    assert(state != nullptr);
  }

  void LuaRegistryReference::putOnTop(std::function<void (lua_State *)> callback) {
    this->handle.get(callback);
  }

  void LuaRegistryReference::setValue(std::function<void (lua_State *)> gen) {
    this->handle.set(gen);
  }
}