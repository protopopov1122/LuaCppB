#include "luacppb/Table.h"

namespace LuaCppB {


   LuaTableReference::LuaTableReference(std::map<std::string, std::shared_ptr<LuaData>> &table, const std::string &key)
      : table(table), key(key) {}

  LuaTableReference & LuaTableReference::operator=(const LuaValue &value) {
    this->table[this->key] = std::make_shared<LuaValue>(value);
    return *this;
  }
  
  LuaTableReference & LuaTableReference::operator=(const LuaTable &value) {
    this->table[this->key] = std::make_shared<LuaTable>(value);
    return *this;
  }

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

  LuaTable &LuaTable::put(const std::string &key, const LuaTable &table) {
    this->contents[key] = std::make_shared<LuaTable>(table);
    return *this;
  }

  LuaTableReference LuaTable::operator[](const std::string &key) {
    return LuaTableReference(this->contents, key);
  }
}