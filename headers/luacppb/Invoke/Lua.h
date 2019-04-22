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

#ifndef LUACPPB_INVOKE_LUA_H_
#define LUACPPB_INVOKE_LUA_H_

#include "luacppb/Base.h"
#include "luacppb/Value/Native.h"
#include "luacppb/Reference/Registry.h"
#include "luacppb/Core/Runtime.h"
#include "luacppb/Core/Stack.h"
#include "luacppb/Invoke/Error.h"
#include <type_traits>
#include <tuple>
#include <vector>
#include <algorithm>

namespace LuaCppB::Internal {

  template <typename P, typename ... A>
  struct LuaFunctionArgument_Impl {};

  template <typename P>
  struct LuaFunctionArgument_Impl<P> {
    static void push(lua_State *, LuaCppRuntime &);
  };

  template <typename P, typename B, typename ... A>
  struct LuaFunctionArgument_Impl<P, B, A...> {
    static void push(lua_State *, LuaCppRuntime &, B &&, A &&...);
  };

  template <typename P, typename ... A>
  struct LuaFunctionArgument {
    static void push(lua_State *, LuaCppRuntime &, A &&...);
  };

  class LuaFunctionContinuation {
  public:
    virtual ~LuaFunctionContinuation() = default;
    virtual int call(lua_State *, LuaCppRuntime &, LuaError &, std::vector<LuaValue> &) = 0;
  };

#ifdef LUACPPB_COROUTINE_SUPPORT

  class LuaFunctionContinuationHandle {
  public:
    LuaFunctionContinuationHandle(std::unique_ptr<LuaFunctionContinuation>, LuaCppRuntime &, int);
    
    void getResult(lua_State *state, std::vector<LuaValue> &);
    std::shared_ptr<LuaRuntimeInfo> &getRuntime();
    LuaFunctionContinuation &getContinuation();
    static int fnContinuation(lua_State *, int, lua_KContext);
  private:
    std::unique_ptr<LuaFunctionContinuation> cont;
    std::shared_ptr<LuaRuntimeInfo> runtime;
    int top;
  };

#endif

  class LuaFunctionCall {
  public:
    template <typename ... A>
    static LuaError call(lua_State *, int, LuaCppRuntime &, std::vector<LuaValue> &, A &&...);

#ifdef LUACPPB_COROUTINE_SUPPORT
    template <typename ... A>
    static void callK(lua_State *, int, LuaCppRuntime &, std::unique_ptr<LuaFunctionContinuation>, A &&...);

    template <typename ... A>
    static void yieldK(lua_State *, LuaCppRuntime &, std::unique_ptr<LuaFunctionContinuation>, A &&...);
#endif
  };

  template <std::size_t I, typename ... T>
  struct LuaFunctionCallResultTuple_Impl {};

  template <std::size_t I>
  struct LuaFunctionCallResultTuple_Impl<I> {
    static std::tuple<> get(const std::vector<LuaValue> &);
  };

  template <std::size_t I, typename T, typename ... B>
  struct LuaFunctionCallResultTuple_Impl<I, T, B...> {
    static std::tuple<T, B...> get(const std::vector<LuaValue> &);
  };

  template <typename ... T>
  struct LuaFunctionCallResultTuple {
    static std::tuple<T...> get(const std::vector<LuaValue> &);
  };

  template <typename T>
  struct LuaFunctionCallResultGetter {
    static T get(const std::vector<LuaValue> &);
  };

  template <typename ... T>
  struct LuaFunctionCallResultGetter<std::tuple<T...>> {
    static std::tuple<T...> get(const std::vector<LuaValue> &);
  };

  template <typename A, typename B>
  struct LuaFunctionCallResultGetter<std::pair<A, B>> {
    static std::pair<A, B> get(const std::vector<LuaValue> &);
  };

  class LuaFunctionCallResult {
  public:
    LuaFunctionCallResult(std::vector<LuaValue> &, LuaError && = LuaError());
    LuaFunctionCallResult(LuaError &&);
    LuaFunctionCallResult(const LuaFunctionCallResult &) = delete;
    LuaFunctionCallResult &operator=(const LuaFunctionCallResult &) = delete;

    LuaFunctionCallResult &status(LuaStatusCode &);
    LuaFunctionCallResult &error(LuaError &);
    bool hasError() const;
    bool operator==(LuaStatusCode) const;
    bool operator!=(LuaStatusCode) const;
    void push(lua_State *);
    LuaValue value(std::size_t = 0) const;
    const std::vector<LuaValue> &values() const;
    LuaValue operator*() const;

    template <typename T>
    T get() const;

    template <typename T>
    operator T() const;
  private:
    std::vector<LuaValue> result;
    LuaError errorVal;
  };

  template<typename H, typename T>
  struct LuaLambdaBuilder : public LuaLambdaBuilder<H, decltype(&T::operator())> {};

  template<typename H, typename R, typename ... A>
  struct LuaLambdaBuilder<H, R(A...)> {
    using FunctionType = std::function<R(A...)>;
    static FunctionType create(H &&);
  };

  template<typename H, typename R, typename ... A>
  struct LuaLambdaBuilder<H, R(*)(A...)> {
    using FunctionType = std::function<R(A...)>;
    static FunctionType create(H &&);
  };

  template<typename H, typename C, typename R, typename ... A>
  struct LuaLambdaBuilder<H, R (C::*)(A...) const> {
    using FunctionType = std::function<R(A...)>;
    static FunctionType create(H &&);
  };

  template<typename H, typename C, typename R, typename ... A>
  struct LuaLambdaBuilder<H, R (C::*)(A...)> {
    using FunctionType = std::function<R(A...)>;
    static FunctionType create(H &&);
  };

  template <typename H>
  class LuaLambdaHandle {
  public:
    LuaLambdaHandle(H &&);
    LuaLambdaHandle(const LuaLambdaHandle &) = delete;
    LuaLambdaHandle &operator=(const LuaLambdaHandle &) = delete;

    template <typename T>
    operator T();
  private:
    H handle;
  };
}

#include "luacppb/Invoke/Impl/Lua.h"

#endif
