#ifndef LUACPPB_INVOKE_ARGRET_H_
#define LUACPPB_INVOKE_ARGRET_H_

#include "luacppb/Base.h"
#include "luacppb/Core/Runtime.h"
#include "luacppb/Core/Stack.h"
#include "luacppb/Core/State.h"
#include <type_traits>
#include <tuple>
#include <array>

namespace LuaCppB::Internal {

	template <std::size_t I, typename T, typename E = void>
	struct NativeFunctionArgument {
		static T get(lua_State *, LuaCppRuntime &, LuaValue &);

		static constexpr bool Virtual = false;
	};

	template <std::size_t I, typename T>
	struct NativeFunctionArgument<I, T, typename std::enable_if<std::is_same<T, LuaState>::value>::type> {
		static T get(lua_State *, LuaCppRuntime &, LuaValue &);

		static constexpr bool Virtual = true;
	};

	template <std::size_t I, typename T>
	struct NativeFunctionArgument<I, T, typename std::enable_if<std::is_same<T, LuaReferenceHandle>::value>::type> {
		static T get(lua_State *, LuaCppRuntime &, LuaValue &);

		static constexpr bool Virtual = false;
	};

	template <std::size_t Index, std::size_t Offset, std::size_t Count, typename ... T>
	struct WrappedFunctionArguments_Impl {};

	template <std::size_t Index, std::size_t Offset, std::size_t Count>
	struct WrappedFunctionArguments_Impl<Index, Offset, Count> {
		static void get(lua_State *, std::array<LuaValue, Count> &);
	};

	template <std::size_t Index, std::size_t Offset, std::size_t Count, typename T, typename ... Ts>
	struct WrappedFunctionArguments_Impl<Index, Offset, Count, T, Ts...> {
		static void get(lua_State *, std::array<LuaValue, Count> &);
	};

	template <std::size_t Offset, typename ... T>
	struct WrappedFunctionArguments {
		static void get(lua_State *, std::array<LuaValue, sizeof...(T)> &);
	};

	template <typename P, typename T, typename E = void>
	struct NativeFunctionResult {
		static int set(lua_State *, LuaCppRuntime &, T &&);
	};

	template <typename P, typename T>
	struct NativeFunctionResult<P, T, typename std::enable_if<is_instantiation<std::pair, T>::value>::type> {
		static int set(lua_State *, LuaCppRuntime &, T &&);
	};

	template <typename P, std::size_t I, typename T>
	struct NativeFunctionResult_Tuple {
		static void push(lua_State *, LuaCppRuntime &, T &&);
	};

	template <typename P, typename T>
	struct NativeFunctionResult<P, T, typename std::enable_if<is_instantiation<std::tuple, T>::value>::type> {
		static int set(lua_State *, LuaCppRuntime &, T &&);
	};
	
	template <std::size_t Index, std::size_t Offset, std::size_t Count, typename ... Ts>
	struct NativeFunctionArgumentsTuple_Impl {};

	template <std::size_t Index, std::size_t Offset, std::size_t Count>
	struct NativeFunctionArgumentsTuple_Impl<Index, Offset, Count> {
		static std::tuple<> value(lua_State *, LuaCppRuntime &, std::array<LuaValue, Count> &);
	};

	template <std::size_t Index, std::size_t Offset, std::size_t Count, typename T, typename ... Ts>
	struct NativeFunctionArgumentsTuple_Impl<Index, Offset, Count, T, Ts...> {
		static std::tuple<T, Ts...> value(lua_State *, LuaCppRuntime &, std::array<LuaValue, Count> &);
	};
	
	template <std::size_t Offset, typename ... A>
	struct NativeFunctionArgumentsTuple {
		static std::tuple<A...> value(lua_State *, LuaCppRuntime &, std::array<LuaValue, sizeof...(A)> &);
	};
}

#include "luacppb/Invoke/Impl/ArgRet.h"

#endif