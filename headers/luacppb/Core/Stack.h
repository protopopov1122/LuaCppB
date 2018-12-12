#ifndef LUACPPB_CORE_STACK_H_
#define LUACPPB_CORE_STACK_H_

#include "luacppb/Base.h"
#include "luacppb/Value/Value.h"
#include <string>
#include <type_traits>

namespace LuaCppB {

  class LuaStack {
   public:
    LuaStack(lua_State *);

    int getTop() const;
    LuaType getType(int = -1) const;
    std::optional<LuaValue> get(int = -1) const;

    void pop(unsigned int = 1);
    void copy(int);

    void push();
    void push(bool);
    void push(const std::string &);
    void push(int (*)(lua_State *), int = 0);
    void pushTable();

    template <typename T>
    typename std::enable_if<std::is_integral<T>::value && !std::is_same<T, bool>::value>::type push(T value) {
      lua_pushinteger(this->state, static_cast<lua_Integer>(value));
    }

    template <typename T>
    typename std::enable_if<std::is_floating_point<T>::value>::type push(T value) {
      lua_pushnumber(this->state, static_cast<lua_Number>(value));
    }

    template <typename T>
    void push(T *pointer) {
      lua_pushlightuserdata(this->state, reinterpret_cast<void *>(pointer));
    }

    template <typename T>
    T *push() {
      return reinterpret_cast<T *>(lua_newuserdata(state, sizeof(T)));
    }
   private:
    lua_State *state;
  };
}

#endif