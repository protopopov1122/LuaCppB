#include "luacppb/Core/Stack.h"
#include "luacppb/Core/Error.h"

namespace LuaCppB {

  LuaStack::LuaStack(lua_State *state)
    : state(state) {
    if (state == nullptr) {
      throw LuaCppBError("Lua state can't be null", LuaCppBErrorCode::InvalidState);
    }
  }

  int LuaStack::getTop() const {
    return lua_gettop(this->state);
  }

  LuaType LuaStack::getType(int index) const {
    return static_cast<LuaType>(lua_type(this->state, index));
  }

  std::optional<LuaValue> LuaStack::get(int index) const {
    return LuaValue::peek(this->state, index);
  }

  void LuaStack::pop(unsigned int count) {
    lua_pop(this->state, count);
  }

  void LuaStack::copy(int index) {
    lua_pushvalue(this->state, index);
  }

  void LuaStack::push() {
    lua_pushnil(this->state);
  }

  void LuaStack::push(bool value) {
    lua_pushboolean(this->state, static_cast<lua_Integer>(value));
  }

  void LuaStack::push(const std::string &value) {
    lua_pushstring(this->state, value.c_str());
  }

  void LuaStack::push(int (*function)(lua_State *), int upvalues) {
    if (upvalues == 0) {
      lua_pushcfunction(this->state, function);
    } else {
      lua_pushcclosure(this->state, function, upvalues);
    }
  }

  void LuaStack::pushTable() {
    lua_newtable(this->state);
  }

  lua_Integer LuaStack::toInteger(int index) {
    return lua_tointeger(this->state, index);
  }

  lua_Number LuaStack::toNumber(int index) {
    return lua_tonumber(this->state, index);
  }
  
  bool LuaStack::toBoolean(int index) {
    return static_cast<bool>(lua_toboolean(this->state, index));
  }

  std::string LuaStack::toString(int index) {
    return std::string(lua_tostring(this->state, index));
  }

  LuaCFunction LuaStack::toCFunction(int index) {
    return lua_tocfunction(this->state, index);
  }
}