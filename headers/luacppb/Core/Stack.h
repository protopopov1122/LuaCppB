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
    lua_State *pushThread();
    void *pushUserData(std::size_t);

    lua_Integer toInteger(int = -1);
    lua_Number toNumber(int = -1);
    bool toBoolean(int = -1);
    std::string toString(int = -1);
    LuaCFunction toCFunction(int = -1);

    void setField(int, const std::string &);
    void getField(int, const std::string &);

    template <bool R = false>
    void setIndex(int index, int idx) {
      lua_seti(this->state, index, idx);
    }

    template <bool R = false>
    void getIndex(int index, int idx) {
      lua_geti(this->state, index, idx);
    }

    int ref();
    void unref(int);

    bool metatable(const std::string &);
    void setMetatable(int = -1);
    void setMetatable(const std::string &);

    template <typename T>
    T toPointer(int index = -1) {
      return reinterpret_cast<T>(const_cast<void *>(lua_topointer(this->state, index)));
    }

    template <typename T>
    T toUserData(int index = -1) {
      return reinterpret_cast<T>(lua_touserdata(this->state, index));
    }

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

    template <typename T>
    T *checkUserData(int index, const std::string &name) {
      return reinterpret_cast<T *>(const_cast<void *>(luaL_checkudata(this->state, index, name.c_str())));
    }

    template <LuaType T>
    bool is(int index = -1) {
      return false;
    }
   private:
    lua_State *state;
  };
}

#endif