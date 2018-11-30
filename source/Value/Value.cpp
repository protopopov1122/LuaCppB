#include "luacppb/Value/Value.h"
#include "luacppb/Reference/Reference.h"

namespace LuaCppB {

  LuaType LuaValue::getType() const {
    return this->type;
  }

  void LuaValue::push(lua_State *state) const {
    switch (this->type) {
      case LuaType::Number:
        if (this->value.index() == 0) {
          std::get<LuaInteger>(this->value).push(state);
        } else {
          std::get<LuaNumber>(this->value).push(state);
        }
        break;
      case LuaType::Boolean:
        std::get<LuaBoolean>(this->value).push(state);
        break;
      case LuaType::String:
        std::get<LuaString>(this->value).push(state);
        break;
      case LuaType::Function:
        std::get<LuaCFunction>(this->value).push(state);
        break;
      case LuaType::Table:
        std::get<LuaTable>(this->value).push(state);
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
      value = LuaValue::create<LuaCFunction_ptr>(lua_tocfunction(state, index));
    } else if (lua_istable(state, index)) {
      value = LuaValue(LuaTable(state, index));
    }
    return value;
  }
}