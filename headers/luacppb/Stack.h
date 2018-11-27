#ifndef LUACPPB_STACK_H_
#define LUACPPB_STACK_H_

#include "luacppb/Base.h"
#include "luacppb/Value.h"
#include <cstdlib>
#include <functional>

namespace LuaCppB {

  class LuaReferenceHandle;

  class LuaStack {
   public:
    LuaStack(lua_State *);

    std::size_t getTop() const;
    void setTop(std::size_t);
    void insert(int);
    void pop(std::size_t = 1);
    void remove(int);
    void replace(int);
    void copy(int);
    void move(int, int);

    LuaType getType(int = -1) const;
    void execute(std::function<void(lua_State *)>);
    LuaReferenceHandle operator[](int);

    template <typename T>
    T get(int index = -1) {
      return T::get(this->state, index);
    }

    template <typename T>
    void push(T value) {
      value.push(this->state);
    }
   private:
    lua_State *state;
  };
}

#endif