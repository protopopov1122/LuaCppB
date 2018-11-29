#include "luacppb/Value/Value.h"
#include "luacppb/Reference/Reference.h"

namespace LuaCppB {

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

  LuaTableBase::LuaTableBase()
    : handle()  {}

  LuaTableBase::LuaTableBase(lua_State *state, int index)
    : handle(state, index) {}

  LuaTableBase::LuaTableBase(const LuaTableBase &base)
    : handle(base.handle) {}

  void LuaTableBase::push(lua_State *state) const {
    handle.get([&](lua_State *handleState) {
      assert(state == handleState);
      lua_pushvalue(state, -1);
    });
  }

  LuaTableBase LuaTableBase::get(lua_State *state, int index) {
    return LuaTableBase(state, index);
  }

  LuaTableBase LuaTableBase::create(lua_State *state) {
    lua_newtable(state);
    LuaTableBase table(state);
    lua_pop(state, 1);
    return table;
  }

  template <>
  LuaReferenceHandle LuaTableBase::convert<LuaReferenceHandle>() {
    LuaReferenceHandle handle;
    this->handle.get([&](lua_State *state) {
      handle = LuaReferenceHandle(state, std::make_unique<LuaRegistryReference>(state));
    });
    return handle;
  }
}