#include "luacppb/Table.h"

namespace LuaCppB {

  void LuaTable::push(lua_State *state) const {
    lua_newtable(state);
    for (auto it = this->contents.begin(); it != this->contents.end(); ++it) {
      const std::string &key = it->first;
      LuaData &value = *it->second;
      value.push(state);
      lua_setfield(state, -2, key.c_str());
    }
  }

  LuaTable &LuaTable::put(const std::string &key, LuaValue value) {
    this->contents[key] = std::make_shared<LuaValue>(value);
    return *this;
  }

  LuaTable &LuaTable::put(const std::string &key, LuaTable &table) {
    this->contents[key] = std::make_shared<LuaTable>(table);
    return *this;
  }
}