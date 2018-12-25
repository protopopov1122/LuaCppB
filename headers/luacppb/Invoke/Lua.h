#ifndef LUACPPB_INVOKE_LUA_H_
#define LUACPPB_INVOKE_LUA_H_

#include "luacppb/Base.h"
#include "luacppb/Value/Native.h"
#include "luacppb/Reference/Registry.h"
#include "luacppb/Core/Runtime.h"
#include "luacppb/Core/Stack.h"
#include <type_traits>
#include <tuple>
#include <vector>
#include <algorithm>

namespace LuaCppB {

  template <typename ... A>
  struct LuaFunctionArgument_Impl {};

  template <>
  struct LuaFunctionArgument_Impl<> {
    static void push(lua_State *state, LuaCppRuntime &runtime) {}
  };

  template <typename B, typename ... A>
  struct LuaFunctionArgument_Impl<B, A...> {
    static void push(lua_State *state, LuaCppRuntime &runtime, B &arg, A &... args) {
      LuaNativeValue::push<B>(state, runtime, arg);
      LuaFunctionArgument_Impl<A...>::push(state, runtime, args...);
    }
  };

  template <typename ... A>
  struct LuaFunctionArgument {
    static void push(lua_State *state, LuaCppRuntime &runtime, A &... args) {
      LuaFunctionArgument_Impl<A...>::push(state, runtime, args...);
    }
  };

  class LuaFunctionContinuation {
   public:
    virtual ~LuaFunctionContinuation() = default;
    virtual int call(LuaStatusCode, std::vector<LuaValue> &) = 0;
  };

  class LuaFunctionContinuationHandle {
   public:
    LuaFunctionContinuationHandle(std::unique_ptr<LuaFunctionContinuation> cont, LuaCppRuntime &runtime, int top)
      : cont(std::move(cont)), runtime(runtime), top(top) {}
    
    void getResult(lua_State *state, std::vector<LuaValue> &result) {
      LuaStack stack(state);
      int results = stack.getTop() - top;
      while (results-- > 0) {
        result.push_back(LuaValue::peek(state).value());
        stack.pop();
      }
      std::reverse(result.begin(), result.end());
    }

    LuaCppRuntime &getRuntime() {
      return this->runtime;
    }

    LuaFunctionContinuation &getContinuation() {
      return *this->cont;
    }

    static int fnContinuation(lua_State *state, int status, lua_KContext ctx) {
      std::unique_ptr<LuaFunctionContinuationHandle> handle(reinterpret_cast<LuaFunctionContinuationHandle *>(ctx));
      std::vector<LuaValue> result;
      handle->getResult(state, result);
      return handle->getContinuation().call(static_cast<LuaStatusCode>(status), result);
    }
   private:
    std::unique_ptr<LuaFunctionContinuation> cont;
    LuaCppRuntime &runtime;
    int top;
  };

  class LuaFunctionCall {
   public:
    template <typename ... A>
    static LuaStatusCode call(lua_State *state, int index, LuaCppRuntime &runtime, std::vector<LuaValue> &result, A &... args) {
      LuaStack stack(state);
      int top = stack.getTop();
      stack.copy(index);
      LuaFunctionArgument<A...>::push(state, runtime, args...);
      int status = lua_pcall(state, sizeof...(args), LUA_MULTRET, 0);
      int results = stack.getTop() - top;
      while (results-- > 0) {
        result.push_back(LuaValue::peek(state).value());
        stack.pop();
      }
      std::reverse(result.begin(), result.end());
      return static_cast<LuaStatusCode>(status);
    }

    template <typename ... A>
    static void callK(lua_State *state, int index, int top, LuaCppRuntime &runtime, std::unique_ptr<LuaFunctionContinuation> cont, A &... args) {
      LuaStack stack(state);
      stack.copy(index);
      LuaFunctionArgument<A...>::push(state, runtime, args...);
      LuaFunctionContinuationHandle *handle = new LuaFunctionContinuationHandle(std::move(cont), runtime, top);
      lua_KContext ctx = reinterpret_cast<lua_KContext>(handle);
      LuaFunctionContinuationHandle::fnContinuation(state,
        lua_pcallk(state, sizeof...(args), LUA_MULTRET, 0,
                  ctx, &LuaFunctionContinuationHandle::fnContinuation),
        ctx);
    }
  };

  template <std::size_t I, typename ... T>
  struct LuaFunctionCallResultTuple_Impl {};

  template <std::size_t I>
  struct LuaFunctionCallResultTuple_Impl<I> {
    static std::tuple<> get(std::vector<LuaValue> &results) {
      return std::make_tuple();
    }
  };

  template <std::size_t I, typename T, typename ... B>
  struct LuaFunctionCallResultTuple_Impl<I, T, B...> {
    static std::tuple<T, B...> get(std::vector<LuaValue> &results) {
      std::tuple<T> begin = std::make_tuple((I < results.size() ? results.at(I) : LuaValue()).get<T>());
      return std::tuple_cat(begin, LuaFunctionCallResultTuple_Impl<I + 1, B...>::get(results));
    }
  };

  template <typename ... T>
  struct LuaFunctionCallResultTuple {
    static std::tuple<T...> get(std::vector<LuaValue> &results) {
      return LuaFunctionCallResultTuple_Impl<0, T...>::get(results);
    }
  };

  template <typename T>
  struct LuaFunctionCallResultGetter {
    static T get(std::vector<LuaValue> &result) {
      LuaValue ret;
      if (!result.empty()) {
        ret = result.at(0);
      }
      return ret.get<T>();
    }
  };

  template <typename ... T>
  struct LuaFunctionCallResultGetter<std::tuple<T...>> {
    static std::tuple<T...> get(std::vector<LuaValue> &result) {
      return LuaFunctionCallResultTuple<T...>::get(result);
    }
  };

  template <typename A, typename B>
  struct LuaFunctionCallResultGetter<std::pair<A, B>> {
    static std::pair<A, B> get(std::vector<LuaValue> &result) {
      if (result.size() >= 2) {
        return std::make_pair(result.at(0).get<A>(), result.at(1).get<B>());
      } else if (result.size() == 1) {
        return std::make_pair(result.at(0).get<A>(), LuaValue().get<B>());
      } else {
        return std::make_pair(LuaValue().get<A>(), LuaValue().get<B>());
      }
    }
  };

  class LuaFunctionCallResult {
   public:
    LuaFunctionCallResult(std::vector<LuaValue> &result, LuaStatusCode status = LuaStatusCode::Ok)
      : result(result), statusCode(status) {}
    LuaFunctionCallResult(const LuaFunctionCallResult &) = delete;
    LuaFunctionCallResult &operator=(const LuaFunctionCallResult &) = delete;

    LuaFunctionCallResult &status(LuaStatusCode &status) {
      status = this->statusCode;
      return *this;
    }

    template <typename T>
    T get() {
      return LuaFunctionCallResultGetter<T>::get(this->result);
    }

    template <typename T>
    operator T() {
      return this->get<T>();
    }

   private:
    std::vector<LuaValue> result;
    LuaStatusCode statusCode;
  };
}

#endif