#include "luacppb/Value/Value.h"
#include "luacppb/Reference/Reference.h"
#include "luacppb/Error.h"

namespace LuaCppB {

  void LuaNil::push(lua_State *state) const {
    if (state != nullptr) {
      lua_pushnil(state);
    } else {
      throw LuaCppBError("Lua state can't be null", LuaCppBErrorCode::InvalidState);
    }
  }

  LuaNil LuaNil::get(lua_State *state, int index) {
    return LuaNil();
  }

  LuaInteger::LuaInteger() : integer(0) {}

  LuaInteger::LuaInteger(lua_Integer i) : integer(i) {}

  void LuaInteger::push(lua_State *state) const {
    if (state != nullptr) {
      lua_pushinteger(state, this->integer);
    } else {
      throw LuaCppBError("Lua state can't be null", LuaCppBErrorCode::InvalidState);
    }
  }

  LuaInteger LuaInteger::get(lua_State *state, int index) {
    return LuaInteger(lua_tointeger(state, index));
  }

  LuaNumber::LuaNumber() : number(0) {}

  LuaNumber::LuaNumber(lua_Number n) : number(n) {}
  
  void LuaNumber::push(lua_State *state) const {
    if (state != nullptr) {
      lua_pushnumber(state, this->number);
    } else {
      throw LuaCppBError("Lua state can't be null", LuaCppBErrorCode::InvalidState);
    }
  }

  LuaNumber LuaNumber::get(lua_State *state, int index) {
    return LuaNumber(lua_tonumber(state, index));
  }

  LuaBoolean::LuaBoolean(bool b) : boolean(b) {}

  void LuaBoolean::push(lua_State *state) const {
    if (state != nullptr) {
      lua_pushboolean(state, static_cast<int>(this->boolean));
    } else {
      throw LuaCppBError("Lua state can't be null", LuaCppBErrorCode::InvalidState);
    }
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
    if (state != nullptr) {
      return LuaString(lua_tostring(state, index));
    } else {
      throw LuaCppBError("Lua state can't be null", LuaCppBErrorCode::InvalidState);
    }
  }

  LuaCFunction::LuaCFunction(LuaCFunction_ptr fn) : function(fn) {}
  
  void LuaCFunction::push(lua_State *state) const {
    if (state != nullptr) {
      lua_pushcfunction(state, this->function);
    } else {
      throw LuaCppBError("Lua state can't be null", LuaCppBErrorCode::InvalidState);
    }
  }

  LuaCFunction LuaCFunction::get(lua_State *state, int index) {
    return LuaCFunction(lua_tocfunction(state, index));
  }

  LuaReferencedValue::LuaReferencedValue()
    : handle()  {}

  LuaReferencedValue::LuaReferencedValue(lua_State *state, int index)
    : handle(state, index) {}

  LuaReferencedValue::LuaReferencedValue(const LuaReferencedValue &base)
    : handle(base.handle) {}

  void LuaReferencedValue::push(lua_State *state) const {
    int ref = -1;
    handle.get([&](lua_State *handleState) {
      if (state == handleState) {
        lua_pushvalue(state, -1);
        ref = luaL_ref(state, LUA_REGISTRYINDEX);
      } else {
        throw LuaCppBError("Reference handler state must match requested state", LuaCppBErrorCode::StateMismatch);
      }
    });
    lua_rawgeti(state, LUA_REGISTRYINDEX, ref);
    luaL_unref(state, LUA_REGISTRYINDEX, ref);
  }

  template <>
  LuaReferenceHandle LuaReferencedValue::convert<LuaReferenceHandle>() {
    LuaReferenceHandle handle;
    this->handle.get([&](lua_State *state) {
      handle = LuaReferenceHandle(state, std::make_unique<LuaRegistryReference>(state));
    });
    return handle;
  }

  LuaTable LuaTable::get(lua_State *state, int index) {
    return LuaTable(state, index);
  }

  LuaTable LuaTable::create(lua_State *state) {
    lua_newtable(state);
    LuaTable table(state);
    lua_pop(state, 1);
    return table;
  }
}