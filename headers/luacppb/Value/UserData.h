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

#ifndef LUACPPB_VALUE_USERDATA_H_
#define LUACPPB_VALUE_USERDATA_H_

#include "luacppb/Reference/Registry.h"
#include "luacppb/Value/Native.h"
#include "luacppb/Invoke/Native.h"
#include <sstream>
#include <atomic>
#include <map>

namespace LuaCppB {

  namespace Internal {

    template <typename T, typename F, typename ... A>
    class CustomUserDataCall : public LuaData {
      using R = typename std::invoke_result<F, T &, A...>::type;
    public:
      CustomUserDataCall(F &&, const std::string &, LuaCppRuntime &);
      void push(lua_State *state) const;
    private:
      static int call(F &, const std::string &, LuaCppRuntime &, lua_State *);
      static int userdata_closure(lua_State *);

      F invocable;
      std::string className;
      LuaCppRuntime &runtime;
    };

    template <typename C, typename T>
    struct CustomUserDataCallBuilder : public CustomUserDataCallBuilder<C, decltype(&T::operator())> {
      using FunctionType = typename CustomUserDataCallBuilder<C, decltype(&T::operator())>::FunctionType;
      using Type = typename CustomUserDataCallBuilder<C, decltype(&T::operator())>::Type;
      static std::shared_ptr<Type> create(FunctionType &&, const std::string &, LuaCppRuntime &);
    };

    template <typename T, typename R, typename ... A>
    struct CustomUserDataCallBuilder<T, R(T &, A...)> {
      using FunctionType = std::function<R(T &, A...)>;
      using Type = CustomUserDataCall<T, FunctionType, A...>;
    };

    template<typename T, typename R, typename ... A>
    struct CustomUserDataCallBuilder<T, R(*)(T &, A...)> {
      using FunctionType = std::function<R(T &, A...)>;
      using Type = CustomUserDataCall<T, FunctionType, A...>;
    };

    template<typename T, typename C, typename R, typename ... A>
    struct CustomUserDataCallBuilder<T, R (C::*)(T &, A...) const> {
      using FunctionType = std::function<R(T &, A...)>;
      using Type = CustomUserDataCall<T, FunctionType, A...>;
    };

    template<typename T, typename C, typename R, typename ... A>
    struct CustomUserDataCallBuilder<T, R (C::*)(T &, A...)> {
      using FunctionType = std::function<R(T &, A...)>;
      using Type = CustomUserDataCall<T, FunctionType, A...>;
    };
  }

  enum class LuaMetamethod {
    GC,
    Index,
    NewIndex,
    Call,
    ToString,
    Length,
    Pairs,
    IPairs,
    //Mathematical
    UnaryMinus,
    Add,
    Subtract,
    Multiply,
    Divide,
    FloorDivide,
    Modulo,
    Power,
    Concat,
    // Bitwise
    BitwiseAnd,
    BitwiseOr,
    BitwiseXor,
    BitwiseNot,
    ShiftLeft,
    ShiftRight,
    // Equivalence
    Equals,
    LessThan,
    LessOrEqual
  };

  template <typename T>
  class CustomUserData : public LuaData {
   public:
    CustomUserData(T *, Internal::LuaSharedRegistryHandle);
    
    operator T*();
    operator T&();

    T *get();
	  void push(lua_State *) const override;
   private:
    T *pointer;
    Internal::LuaSharedRegistryHandle handle;
  };

  template <typename T>
  class CustomUserDataClass {
   public:
    CustomUserDataClass(LuaCppRuntime &);

    void setDefaultConstructor(std::function<void(T &)> &&);

    template <typename S>
    CustomUserData<T> create(S &, const std::function<void(T &)> &&) const;

    template <typename S>
    CustomUserData<T> create(S &) const;

    template <typename F>
    void bind(const std::string &, F &&);

    template <typename F>
    bool bind(LuaMetamethod, F &&);
   private:
    LuaCppRuntime &runtime;
    std::string className;
    std::function<void(T &)> constructor;
    std::map<std::string, std::shared_ptr<LuaData>> methods;

    static std::atomic<uint64_t> nextIdentifier;
    static std::map<LuaMetamethod, std::string> metamethods;
  };
}

#include "luacppb/Value/Impl/UserData.h"

#endif
