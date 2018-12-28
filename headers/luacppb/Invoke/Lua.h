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

namespace LuaCppB {

  namespace Internal {

    template <typename P, typename ... A>
    struct LuaFunctionArgument_Impl {};

    template <typename P>
    struct LuaFunctionArgument_Impl<P> {
      static void push(lua_State *state, LuaCppRuntime &runtime) {}
    };

    template <typename P, typename B, typename ... A>
    struct LuaFunctionArgument_Impl<P, B, A...> {
      static void push(lua_State *state, LuaCppRuntime &runtime, B &arg, A &... args) {
        P::push(state, runtime, arg);
        LuaFunctionArgument_Impl<P, A...>::push(state, runtime, args...);
      }
    };

    template <typename P, typename ... A>
    struct LuaFunctionArgument {
      static void push(lua_State *state, LuaCppRuntime &runtime, A &... args) {
        LuaFunctionArgument_Impl<P, A...>::push(state, runtime, args...);
      }
    };

    class LuaFunctionContinuation {
    public:
      virtual ~LuaFunctionContinuation() = default;
      virtual int call(lua_State *, LuaCppRuntime &, LuaError, std::vector<LuaValue> &) = 0;
    };

    class LuaFunctionContinuationHandle {
    public:
      LuaFunctionContinuationHandle(std::unique_ptr<LuaFunctionContinuation> cont, LuaCppRuntime &runtime, int top)
        : cont(std::move(cont)), runtime(runtime.getRuntimeInfo()), top(top) {}
      
      void getResult(lua_State *state, std::vector<LuaValue> &result) {
        Internal::LuaStack stack(state);
        int results = stack.getTop() - top;
        while (results-- > 0) {
          result.push_back(LuaValue::peek(state).value());
          stack.pop();
        }
        std::reverse(result.begin(), result.end());
      }

      std::shared_ptr<LuaRuntimeInfo> &getRuntime() {
        return this->runtime;
      }

      LuaFunctionContinuation &getContinuation() {
        return *this->cont;
      }

      static int fnContinuation(lua_State *state, int statusCode, lua_KContext ctx) {
        std::unique_ptr<LuaFunctionContinuationHandle> handle(reinterpret_cast<LuaFunctionContinuationHandle *>(ctx));
        LuaState luaState(state, handle->getRuntime());
        LuaStatusCode status = static_cast<LuaStatusCode>(statusCode);
        std::vector<LuaValue> result;
        if (status == LuaStatusCode::Ok || status == LuaStatusCode::Yield) {
          handle->getResult(state, result);
          return handle->getContinuation().call(state, luaState, LuaError(status), result);
        } else {
          LuaStack stack(state);
          std::optional<LuaValue> value = stack.get();
          stack.pop();
          LuaError error(status, value.value_or(LuaValue()));
          return handle->getContinuation().call(state, luaState, error, result);
        }
      }
    private:
      std::unique_ptr<LuaFunctionContinuation> cont;
      std::shared_ptr<LuaRuntimeInfo> runtime;
      int top;
    };

    class LuaFunctionCall {
    public:
      template <typename ... A>
      static LuaError call(lua_State *state, int index, LuaCppRuntime &runtime, std::vector<LuaValue> &result, A &... args) {
        Internal::LuaStack stack(state);
        int top = stack.getTop();
        stack.copy(index);
        LuaFunctionArgument<Internal::LuaNativeValue, A...>::push(state, runtime, args...);
        int status = lua_pcall(state, sizeof...(args), LUA_MULTRET, 0);
        if (status == static_cast<int>(LuaStatusCode::Ok)) {
          int results = stack.getTop() - top;
          while (results-- > 0) {
            result.push_back(LuaValue::peek(state).value());
            stack.pop();
          }
          std::reverse(result.begin(), result.end());
          return LuaError();
        } else {
          std::optional<LuaValue> value = stack.get();
          stack.pop();
          return LuaError(static_cast<LuaStatusCode>(status), value.value_or(LuaValue()));
        }
      }

      template <typename ... A>
      static void callK(lua_State *state, int index, LuaCppRuntime &runtime, std::unique_ptr<LuaFunctionContinuation> cont, A &... args) {
        Internal::LuaStack stack(state);
        int top = stack.getTop();
        stack.copy(index);
        LuaFunctionArgument<Internal::LuaNativeValue, A...>::push(state, runtime, args...);
        LuaFunctionContinuationHandle *handle = new LuaFunctionContinuationHandle(std::move(cont), runtime, top);
        lua_KContext ctx = reinterpret_cast<lua_KContext>(handle);
        LuaFunctionContinuationHandle::fnContinuation(state,
          lua_pcallk(state, sizeof...(args), LUA_MULTRET, 0,
                    ctx, &LuaFunctionContinuationHandle::fnContinuation),
          ctx);
      }

      template <typename ... A>
      static void yieldK(lua_State *state, LuaCppRuntime &runtime, std::unique_ptr<LuaFunctionContinuation> cont, A &... args) {
        Internal::LuaStack stack(state);
        int top = stack.getTop();
        LuaFunctionArgument<Internal::LuaNativeValue, A...>::push(state, runtime, args...);
        LuaFunctionContinuationHandle *handle = new LuaFunctionContinuationHandle(std::move(cont), runtime, top);
        lua_KContext ctx = reinterpret_cast<lua_KContext>(handle);
        LuaFunctionContinuationHandle::fnContinuation(state,
          lua_yieldk(state, sizeof...(args),
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
      LuaFunctionCallResult(std::vector<LuaValue> &result, LuaError err = LuaError())
        : result(result), errorVal(err) {}
      LuaFunctionCallResult(LuaError err)
        : result(), errorVal(err) {}
      LuaFunctionCallResult(const LuaFunctionCallResult &) = delete;
      LuaFunctionCallResult &operator=(const LuaFunctionCallResult &) = delete;

      LuaFunctionCallResult &status(LuaStatusCode &status) {
        status = this->errorVal.getStatus();
        return *this;
      }

      LuaFunctionCallResult &error(LuaError &error) {
        error = this->errorVal;
        return *this;
      }

      bool hasError() const {
        return this->errorVal.hasError();
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
      LuaError errorVal;
    };
  }
}

#endif