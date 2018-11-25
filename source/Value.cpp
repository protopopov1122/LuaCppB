#include "luacppb/Value.h"

namespace LuaCppB {

  LuaType LuaValue::getType() const {
    return this->type;
  }

  void LuaValue::push(lua_State *state) const {
    switch (this->type) {
      case LuaType::Number:
        if (this->value.index() == 0) {
          lua_pushinteger(state, std::get<lua_Integer>(this->value));
        } else {
          lua_pushnumber(state, std::get<lua_Number>(this->value));
        }
        break;
      case LuaType::Boolean:
        lua_pushboolean(state, static_cast<int>(std::get<bool>(this->value)));
        break;
      case LuaType::String:
        lua_pushstring(state, std::get<std::string>(this->value).c_str());
        break;
      case LuaType::Function:
        lua_pushcfunction(state, std::get<LuaCFunction>(this->value));
        break;
      default:
        break;
    } 
  }

  std::optional<LuaValue> LuaValue::peek(lua_State *state, lua_Integer index) {
    std::optional<LuaValue> value;
    if (lua_isinteger(state, index)) {
      value = LuaValue::create<lua_Integer>(lua_tointeger(state, index));
    } else if (lua_isnumber(state, index)) {
      value = LuaValue::create<lua_Number>(lua_tonumber(state, index));
    } else if (lua_isboolean(state, index)) {
      value = LuaValue::create<bool>(lua_toboolean(state, index));
    } else if (lua_isstring(state, index)) {
      value = LuaValue::create<const char *>(lua_tostring(state, index));
    } else if (lua_iscfunction(state, index)) {
      value = LuaValue::create<LuaCFunction>(lua_tocfunction(state, index));
    }
    return value;
  }
}