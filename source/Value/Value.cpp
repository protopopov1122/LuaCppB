#include "luacppb/Value/Value.h"
#include "luacppb/Reference/Reference.h"
#include "luacppb/Core/Error.h"

namespace LuaCppB {

  LuaType LuaValue::getType() const noexcept {
    return this->type;
  }

  void LuaValue::push(lua_State *state) const {
    switch (this->type) {
      case LuaType::Number:
        assert(this->value.index() == 0 || this->value.index() == 1);
        if (this->value.index() == 0) {
          std::get<LuaInteger>(this->value).push(state);
        } else {
          std::get<LuaNumber>(this->value).push(state);
        }
        break;
      case LuaType::Boolean:
        assert(this->value.index() == 2);
        std::get<LuaBoolean>(this->value).push(state);
        break;
      case LuaType::String:
        assert(this->value.index() == 3);
        std::get<LuaString>(this->value).push(state);
        break;
      case LuaType::Function:
        assert(this->value.index() == 4);
        std::get<LuaCFunction>(this->value).push(state);
        break;
      case LuaType::Table:
        assert(this->value.index() == 5);
        std::get<LuaTable>(this->value).push(state);
        break;
      default:
        break;
    } 
  }

  std::optional<LuaValue> LuaValue::peek(lua_State *state, lua_Integer index) {
    if (state == nullptr) {
      throw LuaCppBError("Lua state can't be null", LuaCppBErrorCode::InvalidState);
    }
    if (index > lua_gettop(state)) {
      throw LuaCppBError("Lua stack overflow", LuaCppBErrorCode::StackOverflow);
    }
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