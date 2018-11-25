#ifndef LUACPPB_TABLE_H_
#define LUACPPB_TABLE_H_

#include "luacppb/Value.h"
#include "luacppb/Function.h"
#include <map>
#include <string>
#include <memory>
#include <type_traits>

namespace LuaCppB {

  class LuaTable : public LuaData {
   public:
    void push(lua_State *) const override;
    LuaTable &put(const std::string &, LuaValue);
    LuaTable &put(const std::string &, LuaTable &);
    LuaTable &put(const std::string &, LuaTable &&);

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