/*
  SPDX short identifier: MIT

  Copyright 2018-2019 Jevgēnijs Protopopovs

  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"),
  to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
  and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
  IN THE SOFTWARE.
*/

#include "luacppb/Core/Stack.h"
#include "luacppb/Core/Error.h"

namespace LuaCppB::Internal {

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

  bool LuaStack::check(int count) {
    return static_cast<bool>(lua_checkstack(this->state, count));
  }

  void LuaStack::move(lua_State *state, int count) {
    lua_xmove(this->state, state, count);
  }

  void LuaStack::remove(int index) {
    lua_remove(this->state, index);
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

  void LuaStack::push(LuaData &value) {
    value.push(this->state);
  }

  void LuaStack::pushTable() {
    lua_newtable(this->state);
  }

  lua_State *LuaStack::pushThread() {
    return lua_newthread(this->state);
  }

  void LuaStack::pushThread(lua_State *thread) {
    lua_pushthread(thread);
  }

  void *LuaStack::pushUserData(std::size_t sz) {
    return lua_newuserdata(this->state, sz);
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
    const char *raw = lua_tostring(this->state, index);
    return std::string(raw != nullptr ? raw : "");
  }

  LuaCFunction LuaStack::toCFunction(int index) {
    return lua_tocfunction(this->state, index);
  }

  lua_State *LuaStack::toThread(int index) {
    return lua_tothread(this->state, index);
  }

  bool LuaStack::compare(int idx1, int idx2, LuaCompareOperation op) {
    return static_cast<bool>(lua_compare(this->state, idx1, idx2, static_cast<int>(op)));
  }

  void LuaStack::setField(int index, const std::string &key) {
    lua_setfield(this->state, index, key.c_str());
  }

  void LuaStack::getField(int index, const std::string &key) {
    lua_getfield(this->state, index, key.c_str());
  }
  
  bool LuaStack::metatable(const std::string &name) {
    bool res = static_cast<bool>(luaL_newmetatable(this->state, name.c_str()));
    if (res) {
      this->push(std::string("__name"));
      this->push(name);
      lua_rawset(this->state, -3);
    }
    return res;
  }

  void LuaStack::getMetatable(int index) {
    lua_getmetatable(this->state, index);
  }

  void LuaStack::setMetatable(int index) {
    lua_setmetatable(this->state, index);
  }

  void LuaStack::setMetatable(const std::string &name) {
    luaL_setmetatable(this->state, name.c_str());
  }

  int LuaStack::ref() {
    return luaL_ref(this->state, LUA_REGISTRYINDEX);
  }

  void LuaStack::unref(int idx) {
    luaL_unref(this->state, LUA_REGISTRYINDEX, idx);
  }

  bool LuaStack::isNoneOrNil(int index) {
    return static_cast<bool>(lua_isnoneornil(this->state, index));
  }

  bool LuaStack::isInteger(int index) {
    return static_cast<bool>(lua_isinteger(this->state, index));
  }

  bool LuaStack::isCFunction(int index) {
    return static_cast<bool>(lua_iscfunction(this->state, index));
  }
}
