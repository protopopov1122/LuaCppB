#ifndef LUACPPB_INVOKE_ARGRET_H_
#define LUACPPB_INVOKE_ARGRET_H_

#include "luacppb/Base.h"
#include "luacppb/Core/Runtime.h"
#include "luacppb/Core/Stack.h"
#include "luacppb/Core/State.h"
#include <type_traits>
#include <tuple>

namespace LuaCppB::Internal {

	template <std::size_t I, typename T, typename E = void>
	struct NativeFunctionArgument {
		static T get(lua_State *state, LuaCppRuntime &runtime) {
			return LuaValue::peek(state, I).value_or(LuaValue()).get<T>();
		}

		static constexpr bool Virtual = false;
	};

	template <std::size_t I, typename T>
	struct NativeFunctionArgument<I, T, typename std::enable_if<std::is_same<T, LuaState>::value>::type> {
		static T get(lua_State *state, LuaCppRuntime &runtime) {
			return LuaState(state, runtime.getRuntimeInfo());
		}

		static constexpr bool Virtual = true;
	};

	template <typename P, typename T, typename E = void>
	struct NativeFunctionResult {
		static int set(lua_State *state, LuaCppRuntime &runtime, T &value) {
			P::push(state, runtime, value);
			return 1;
		}
	};

	template <typename P, typename T>
	struct NativeFunctionResult<P, T, typename std::enable_if<is_instantiation<std::pair, T>::value>::type> {
		static int set(lua_State *state, LuaCppRuntime &runtime, T &value) {
			P::push(state, runtime, value.first);
			P::push(state, runtime, value.second);
			return 2;
		}
	};

	template <typename P, std::size_t I, typename T>
	struct NativeFunctionResult_Tuple {
		static void push(lua_State *state, LuaCppRuntime &runtime, T &value) {
			Internal::LuaStack stack(state);
			if constexpr (I < std::tuple_size<T>::value) {
				P::push(state, runtime, std::get<I>(value));
				NativeFunctionResult_Tuple<P, I + 1, T>::push(state, runtime, value);
			}
		}
	};

	template <typename P, typename T>
	struct NativeFunctionResult<P, T, typename std::enable_if<is_instantiation<std::tuple, T>::value>::type> {
		static int set(lua_State *state, LuaCppRuntime &runtime, T &value) {
			NativeFunctionResult_Tuple<P, 0, T>::push(state, runtime, value);
			return std::tuple_size<T>::value;
		}
	};
	
	template <std::size_t I, typename ... Ts>
	struct NativeFunctionArgumentsTuple_Impl {};

	template <std::size_t I>
	struct NativeFunctionArgumentsTuple_Impl<I> {
		static std::tuple<> value(lua_State *state, LuaCppRuntime &runtime) {
			return std::make_tuple();
		}
	};

	template <std::size_t I, typename T, typename ... Ts>
	struct NativeFunctionArgumentsTuple_Impl<I, T, Ts...> {
		static std::tuple<T, Ts...> value(lua_State *state, LuaCppRuntime &runtime) {
			if constexpr (!NativeFunctionArgument<I, T>::Virtual) {
				return std::tuple_cat(std::forward_as_tuple(NativeFunctionArgument<I, T>::get(state, runtime)), NativeFunctionArgumentsTuple_Impl<I + 1, Ts...>::value(state, runtime));
			} else {
				return std::tuple_cat(std::forward_as_tuple(NativeFunctionArgument<I, T>::get(state, runtime)), NativeFunctionArgumentsTuple_Impl<I, Ts...>::value(state, runtime));
			}
		};
	};
	
	template <std::size_t S, typename ... A>
	struct NativeFunctionArgumentsTuple {
		static std::tuple<A...> value(lua_State *state, LuaCppRuntime &runtime) {
			return NativeFunctionArgumentsTuple_Impl<S, A...>::value(state, runtime);
		}
	};
}

#endif