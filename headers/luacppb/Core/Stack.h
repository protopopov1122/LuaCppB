#ifndef LUACPPB_CORE_STACK_H_
#define LUACPPB_CORE_STACK_H_

#include "luacppb/Base.h"
#include "luacppb/Value/Value.h"
#include <string>
#include <type_traits>

namespace LuaCppB::Internal {

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
    void setIndex(int, int);

    template <bool R = false>
    void getIndex(int, int);

    int ref();
    void unref(int);

    bool metatable(const std::string &);
    void setMetatable(int = -1);
    void setMetatable(const std::string &);

    template <typename T>
    T toPointer(int = -1);

    template <typename T>
    T toUserData(int = -1);

    template <typename T>
    typename std::enable_if<std::is_integral<T>::value && !std::is_same<T, bool>::value>::type push(T);

    template <typename T>
    typename std::enable_if<std::is_floating_point<T>::value>::type push(T);

    template <typename T>
    void push(T *);

    template <typename T>
    T *push();

    template <typename T>
    T *checkUserData(int, const std::string &);

    template <LuaType T>
    bool is(int = -1);
    
    bool isInteger(int = -1);
    bool isCFunction(int = -1);
   private:
    lua_State *state;
  };
}

#include "luacppb/Core/Impl/Stack.h"

#endif