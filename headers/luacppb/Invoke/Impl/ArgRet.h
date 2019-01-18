#ifndef LUACPPB_INVOKCE_IMPL_ARGRET_H_
#define LUACPPB_INVOKCE_IMPL_ARGRET_H_

#include "luacppb/Invoke/ArgRet.h"

namespace LuaCppB::Internal {

	template <std::size_t I, typename T, typename E>
  T NativeFunctionArgument<I, T, E>::get(lua_State *state, LuaCppRuntime &runtime, LuaValue &value) {
    return value.get<T>();
  }

	template <std::size_t I, typename T>
  T NativeFunctionArgument<I, T, typename std::enable_if<std::is_same<T, LuaState>::value>::type>::get(lua_State *state, LuaCppRuntime &runtime, LuaValue &value) {
    return LuaState(state, runtime.getRuntimeInfo());
  }

	template <std::size_t Index, std::size_t Offset, std::size_t Count>
	void WrappedFunctionArguments_Impl<Index, Offset, Count>::get(lua_State *state, std::array<LuaValue, Count> &array) {}

	template <std::size_t Index, std::size_t Offset, std::size_t Count, typename T, typename ... Ts>
  void WrappedFunctionArguments_Impl<Index, Offset, Count, T, Ts...>::get(lua_State *state, std::array<LuaValue, Count> &array) {
    if constexpr (NativeFunctionArgument<Index, T>::Virtual) {
      WrappedFunctionArguments_Impl<Index, Offset, Count, Ts...>::get(state, array);
    } else {
      array[Index] = LuaValue::peek(state, Offset + Index).value_or(LuaValue());
      WrappedFunctionArguments_Impl<Index + 1, Offset, Count, Ts...>::get(state, array);	
    }
  }

	template <std::size_t Offset, typename ... T>
  void WrappedFunctionArguments<Offset, T...>::get(lua_State *state, std::array<LuaValue, sizeof...(T)> &array) {
    WrappedFunctionArguments_Impl<0, Offset, sizeof...(T), T...>::get(state, array);
  }

	template <typename P, typename T, typename E>
  int NativeFunctionResult<P, T, E>::set(lua_State *state, LuaCppRuntime &runtime, T &&value) {
    int top = lua_gettop(state);
    P::push(state, runtime, value);
    return lua_gettop(state) - top;
  }

	template <typename P, typename T>
  int NativeFunctionResult<P, T, typename std::enable_if<is_instantiation<std::pair, T>::value>::type>::set(lua_State *state, LuaCppRuntime &runtime, T &&value) {
    P::push(state, runtime, value.first);
    P::push(state, runtime, value.second);
    return 2;
  }

	template <typename P, std::size_t I, typename T>
  void NativeFunctionResult_Tuple<P, I, T>::push(lua_State *state, LuaCppRuntime &runtime, T &&value) {
    Internal::LuaStack stack(state);
    if constexpr (I < std::tuple_size<T>::value) {
      P::push(state, runtime, std::get<I>(value));
      NativeFunctionResult_Tuple<P, I + 1, T>::push(state, runtime, std::forward<T>(value));
    }
  }

	template <typename P, typename T>
  int NativeFunctionResult<P, T, typename std::enable_if<is_instantiation<std::tuple, T>::value>::type>::set(lua_State *state, LuaCppRuntime &runtime, T &&value) {
    NativeFunctionResult_Tuple<P, 0, T>::push(state, runtime, std::forward<T>(value));
    return std::tuple_size<T>::value;
  }

	template <std::size_t Index, std::size_t Offset, std::size_t Count>
  std::tuple<> NativeFunctionArgumentsTuple_Impl<Index, Offset, Count>::value(lua_State *state, LuaCppRuntime &runtime, std::array<LuaValue, Count> &array) {
    return std::make_tuple();
  }

	template <std::size_t Index, std::size_t Offset, std::size_t Count, typename T, typename ... Ts>
  std::tuple<T, Ts...> NativeFunctionArgumentsTuple_Impl<Index, Offset, Count, T, Ts...>::value(lua_State *state, LuaCppRuntime &runtime, std::array<LuaValue, Count> &array) {
    if constexpr (!NativeFunctionArgument<Index, T>::Virtual) {
      return std::tuple_cat(std::forward_as_tuple(NativeFunctionArgument<Index, T>::get(state, runtime, array[Index - Offset])),
        NativeFunctionArgumentsTuple_Impl<Index + 1, Offset, Count, Ts...>::value(state, runtime, array));
    } else {
      return std::tuple_cat(std::forward_as_tuple(NativeFunctionArgument<Index, T>::get(state, runtime, array[Index - Offset])),
        NativeFunctionArgumentsTuple_Impl<Index, Offset, Count, Ts...>::value(state, runtime, array));
    }
  }
	
	template <std::size_t Offset, typename ... A>
  std::tuple<A...> NativeFunctionArgumentsTuple<Offset, A...>::value(lua_State *state, LuaCppRuntime &runtime, std::array<LuaValue, sizeof...(A)> &array) {
    return NativeFunctionArgumentsTuple_Impl<Offset, Offset, sizeof...(A), A...>::value(state, runtime, array);
  }
}

#endif