#ifndef LUACPPB_INVOKE_ARGRET_H_
#define LUACPPB_INVOKE_ARGRET_H_

#include "luacppb/Base.h"
#include "luacppb/Core/Runtime.h"
#include "luacppb/Core/Stack.h"
#include "luacppb/Core/State.h"
#include <type_traits>
#include <tuple>
#include <array>
#include <iostream>

namespace LuaCppB::Internal {

	template <typename T, typename E = void>
	struct NativeFunctionArgument {
		static T get(lua_State *state, LuaCppRuntime &runtime, LuaValue &value) {
			return value.get<T>();
		}

		static constexpr bool Virtual = false;
	};

	template <typename T>
	struct NativeFunctionArgument<T, typename std::enable_if<std::is_same<T, LuaState>::value>::type> {
		static T get(lua_State *state, LuaCppRuntime &runtime, LuaValue &value) {
			return LuaState(state, runtime.getRuntimeInfo());
		}

		static constexpr bool Virtual = true;
	};

	template <std::size_t Index, std::size_t Offset, std::size_t Count, typename ... T>
	struct WrappedFunctionArguments_Impl {};

	template <std::size_t Index, std::size_t Offset, std::size_t Count>
	struct WrappedFunctionArguments_Impl<Index, Offset, Count> {
		static void get(lua_State *state, std::array<LuaValue, Count> &array) {}
	};

	template <std::size_t Index, std::size_t Offset, std::size_t Count, typename T, typename ... Ts>
	struct WrappedFunctionArguments_Impl<Index, Offset, Count, T, Ts...> {
		static void get(lua_State *state, std::array<LuaValue, Count> &array) {
			if constexpr (NativeFunctionArgument<T>::Virtual) {
				WrappedFunctionArguments_Impl<Index, Offset, Count, Ts...>::get(state, array);
			} else {
				array[Index] = LuaValue::peek(state, Offset + Index).value_or(LuaValue());
				WrappedFunctionArguments_Impl<Index + 1, Offset, Count, Ts...>::get(state, array);	
			}
		}
	};

	template <std::size_t Offset, typename ... T>
	struct WrappedFunctionArguments {
		static void get(lua_State *state, std::array<LuaValue, sizeof...(T)> &array) {
			WrappedFunctionArguments_Impl<0, Offset, sizeof...(T), T...>::get(state, array);
		}
	};

	template <typename P, typename T, typename E = void>
	struct NativeFunctionResult {
		static int set(lua_State *state, LuaCppRuntime &runtime, T &&value) {
			int top = lua_gettop(state);
			P::push(state, runtime, value);
			return lua_gettop(state) - top;
		}
	};

	template <typename P, typename T>
	struct NativeFunctionResult<P, T, typename std::enable_if<is_instantiation<std::pair, T>::value>::type> {
		static int set(lua_State *state, LuaCppRuntime &runtime, T &&value) {
			P::push(state, runtime, value.first);
			P::push(state, runtime, value.second);
			return 2;
		}
	};

	template <typename P, std::size_t I, typename T>
	struct NativeFunctionResult_Tuple {
		static void push(lua_State *state, LuaCppRuntime &runtime, T &&value) {
			Internal::LuaStack stack(state);
			if constexpr (I < std::tuple_size<T>::value) {
				P::push(state, runtime, std::get<I>(value));
				NativeFunctionResult_Tuple<P, I + 1, T>::push(state, runtime, std::forward<T>(value));
			}
		}
	};

	template <typename P, typename T>
	struct NativeFunctionResult<P, T, typename std::enable_if<is_instantiation<std::tuple, T>::value>::type> {
		static int set(lua_State *state, LuaCppRuntime &runtime, T &&value) {
			NativeFunctionResult_Tuple<P, 0, T>::push(state, runtime, std::forward<T>(value));
			return std::tuple_size<T>::value;
		}
	};
	
	template <std::size_t Index, std::size_t Offset, std::size_t Count, typename ... Ts>
	struct NativeFunctionArgumentsTuple_Impl {};

	template <std::size_t Index, std::size_t Offset, std::size_t Count>
	struct NativeFunctionArgumentsTuple_Impl<Index, Offset, Count> {
		static std::tuple<> value(lua_State *state, LuaCppRuntime &runtime, std::array<LuaValue, Count> &array) {
			return std::make_tuple();
		}
	};

	template <std::size_t Index, std::size_t Offset, std::size_t Count, typename T, typename ... Ts>
	struct NativeFunctionArgumentsTuple_Impl<Index, Offset, Count, T, Ts...> {
		static std::tuple<T, Ts...> value(lua_State *state, LuaCppRuntime &runtime, std::array<LuaValue, Count> &array) {
			if constexpr (!NativeFunctionArgument<T>::Virtual) {
				return std::tuple_cat(std::forward_as_tuple(NativeFunctionArgument<T>::get(state, runtime, array[Index - Offset])),
					NativeFunctionArgumentsTuple_Impl<Index + 1, Offset, Count, Ts...>::value(state, runtime, array));
			} else {
				return std::tuple_cat(std::forward_as_tuple(NativeFunctionArgument<T>::get(state, runtime, array[Index - Offset])),
					NativeFunctionArgumentsTuple_Impl<Index, Offset, Count, Ts...>::value(state, runtime, array));
			}
		};
	};
	
	template <std::size_t Offset, typename ... A>
	struct NativeFunctionArgumentsTuple {
		static std::tuple<A...> value(lua_State *state, LuaCppRuntime &runtime, std::array<LuaValue, sizeof...(A)> &array) {
			return NativeFunctionArgumentsTuple_Impl<Offset, Offset, sizeof...(A), A...>::value(state, runtime, array);
		}
	};
}

#endif