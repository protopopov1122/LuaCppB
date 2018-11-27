#include "luacppb/Value.h"
#include "luacppb/Reference.h"

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
      value = LuaValue(LuaTableBase(state, index));
    }
    return value;
  }

  void LuaNil::push(lua_State *state) const {
    lua_pushnil(state);
  }

  LuaNil LuaNil::get(lua_State *state, int index) {
    return LuaNil();
  }

  LuaInteger::LuaInteger() : integer(0) {}

  LuaInteger::LuaInteger(lua_Integer i) : integer(i) {}

  void LuaInteger::push(lua_State *state) const {
    lua_pushinteger(state, this->integer);
  }

  LuaInteger LuaInteger::get(lua_State *state, int index) {
    return LuaInteger(lua_tointeger(state, index));
  }

  LuaNumber::LuaNumber() : number(0) {}

  LuaNumber::LuaNumber(lua_Number n) : number(n) {}
  
  void LuaNumber::push(lua_State *state) const {
    lua_pushnumber(state, this->number);
  }

  LuaNumber LuaNumber::get(lua_State *state, int index) {
    return LuaNumber(lua_tonumber(state, index));
  }

  LuaBoolean::LuaBoolean(bool b) : boolean(b) {}

  void LuaBoolean::push(lua_State *state) const {
    lua_pushboolean(state, static_cast<int>(this->boolean));
  }

  LuaBoolean LuaBoolean::get(lua_State *state, int index) {
    return LuaBoolean(static_cast<bool>(lua_toboolean(state, index)));
  }

  LuaString::LuaString(const std::string &str) : string(str) {}

  LuaString::LuaString(const char *str) : string(str) {}

  void LuaString::push(lua_State *state) const {
    lua_pushstring(state, this->string.c_str());
  }

  LuaString LuaString::get(lua_State *state, int index) {
    return LuaString(lua_tostring(state, index));
  }

  LuaCFunction::LuaCFunction(LuaCFunction_ptr fn) : function(fn) {}
  
  void LuaCFunction::push(lua_State *state) const {
    lua_pushcfunction(state, this->function);
  }

  LuaCFunction LuaCFunction::get(lua_State *state, int index) {
    return LuaCFunction(lua_tocfunction(state, index));
  }

  LuaTableBase::LuaTableBase() : state(nullptr), ref(0) {}

  LuaTableBase::LuaTableBase(lua_State *state, int index) : state(state) {
    lua_pushvalue(state, index);
    this->ref = luaL_ref(state, LUA_REGISTRYINDEX);
    lua_pop(state, 1);
  }

  void LuaTableBase::push(lua_State *state) const {
    lua_rawgeti(state, LUA_REGISTRYINDEX, this->ref);
  }

  LuaTableBase LuaTableBase::get(lua_State *state, int index) {
    return LuaTableBase(state, index);
  }

  template <>
  LuaReferenceHandle LuaTableBase::convert<LuaReferenceHandle>() {
    lua_rawgeti(this->state, LUA_REGISTRYINDEX, this->ref);
    auto handle = LuaReferenceHandle(this->state, std::make_unique<LuaRegistryReference>(this->state));
    lua_pop(this->state, 1);
    return handle;
  }
}