#ifndef LUACPPB_TABLE_H_
#define LUACPPB_TABLE_H_

#include "luacppb/Value/Value.h"
#include "luacppb/Function.h"
#include <map>
#include <string>
#include <memory>
#include <type_traits>

namespace LuaCppB {

  class LuaTable;

  class LuaTableReference {
   public:
    LuaTableReference(std::map<std::string, std::shared_ptr<LuaData>> &, const std::string &);

    LuaTableReference &operator=(const LuaTable &);
    LuaTableReference &operator=(const LuaValue &);

    template <typename R, typename ... A>
    LuaTableReference &operator=(R (*function)(A...)) {
      this->table[this->key] = std::make_shared<decltype(CFunctionCall(function))>(function);
      return *this;
    }

    template <typename T>
    LuaTableReference &operator=(T value) {
      this->table[this->key] = std::make_shared<LuaValue>(LuaValue::create(value));
      return *this;
    }
   private:
    std::map<std::string, std::shared_ptr<LuaData>> &table;
    std::string key;
  };

  class LuaTable : public LuaData {
   public:
    void push(lua_State *) const override;
    LuaTable &put(const std::string &, LuaValue);
    LuaTable &put(const std::string &, const LuaTable &);
    LuaTable &put(const std::string &, LuaTable &&);
    LuaTableReference operator[](const std::string &);

    template <typename R, typename ... A>
    LuaTable &put(const std::string &key, R (*function)(A...)) {
      this->contents[key] = std::make_shared<decltype(CFunctionCall(function))>(function);
      return *this;
    }

    template <typename C, typename R, typename ... A>
    LuaTable &put(const std::string &key, C *object, R (C::*method)(A...)) {
      this->contents[key] = std::make_shared<decltype(CMethodCall(object, method))>(object, method);
      return *this;
    }

    template <typename T>
    LuaTable &put(const std::string &key, T value) {
      return this->put(key, LuaValue::create<T>(value));
    }
   private:
    std::map<std::string, std::shared_ptr<LuaData>> contents;
  };
}

#endif