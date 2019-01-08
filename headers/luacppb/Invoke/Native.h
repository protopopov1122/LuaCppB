#ifndef LUACPPB_INVOKE_NATIVE_H_
#define LUACPPB_INVOKE_NATIVE_H_

#include "luacppb/Base.h"
#include "luacppb/Invoke/ArgRet.h"
#include "luacppb/Invoke/Method.h"
#include "luacppb/Core/Runtime.h"
#include "luacppb/Core/State.h"
#include "luacppb/Core/Stack.h"
#include "luacppb/Invoke/Descriptor.h"
#include <string>
#include <type_traits>
#include <utility>
#include <functional>
#include <exception>

namespace LuaCppB::Internal {


	class LuaCppBNativeException {
	 public:
		LuaCppBNativeException(std::exception_ptr);

		std::exception_ptr get();
		static void process(lua_State *, std::exception_ptr);
		static void check(lua_State *);
	 private:
	 	static int gc(lua_State *);
	 	static constexpr auto LUACPPB_EXCEPTION_POINTER = "__luacppb_native_exception";
		std::exception_ptr eptr;
	};

	int luacpp_handle_exception(lua_State *, std::exception_ptr);
	
	template <typename P, typename R, typename ... A>
	class NativeFunctionCall : public LuaData {
		using F = R (*)(A...);
	 public:
		NativeFunctionCall(F, LuaCppRuntime &);

		void push(lua_State *) const override;
	 private:
		static int call(F, LuaCppRuntime &, lua_State *);
		static int function_closure(lua_State *);

		F function;
		LuaCppRuntime &runtime;
	};

	template <typename P, typename C, typename R, typename ... A>
	class NativeMethodCall : public LuaData {
		using M = R (C::*)(A...);
		using Mc = R (C::*)(A...) const;
	 public:
		NativeMethodCall(C *, M, LuaCppRuntime &);
		NativeMethodCall(C &, M, LuaCppRuntime &);
		NativeMethodCall(const C *, Mc, LuaCppRuntime &);
		NativeMethodCall(const C &, Mc, LuaCppRuntime &);
		
		void push(lua_State *) const override;
	 private:
	 	static int call(C *, M &&, LuaCppRuntime &, lua_State *);
		static int method_closure(lua_State *);

		C *object;
		M method;
		LuaCppRuntime &runtime;
	};

	template <typename P>
	class NativeMethod {
	 public:
	 	template <typename C, typename R, typename ... A>
	 	static NativeMethodCall<P, C, R, A...> create(C &, R (C::*)(A...), LuaCppRuntime &);
	
	 	template <typename C, typename R, typename ... A>
	 	static NativeMethodCall<P, C, R, A...> create(C *, R (C::*)(A...), LuaCppRuntime &);

	 	template <typename C, typename R, typename ... A>
	 	static NativeMethodCall<P, C, R, A...> create(C &, R (C::*)(A...) const, LuaCppRuntime &);
		
	 	template <typename C, typename R, typename ... A>
	 	static NativeMethodCall<P, C, R, A...> create(C *, R (C::*)(A...) const, LuaCppRuntime &);

	 	template <typename C, typename R, typename ... A>
	 	static NativeMethodCall<P, C, R, A...> create(const C &, R (C::*)(A...) const, LuaCppRuntime &);
		
	 	template <typename C, typename R, typename ... A>
	 	static NativeMethodCall<P, C, R, A...> create(const C *, R (C::*)(A...) const, LuaCppRuntime &);
	};

	template <typename P, typename T, typename ... A>
	class NativeInvocableCall : public LuaData {
		using R = typename std::invoke_result<T, A...>::type;
	 public:
	 	NativeInvocableCall(T &&, LuaCppRuntime &);

		void push(lua_State *) const override;
	 private:
		static int call(T &, LuaCppRuntime &, lua_State *);
		static int invocable_closure(lua_State *);

		T invocable;
		LuaCppRuntime &runtime;
	};

	template<typename P, typename T>
	struct NativeInvocableCallBuilder : public NativeInvocableCallBuilder<P, decltype(&T::operator())> {
		using Type = typename NativeInvocableCallBuilder<P, decltype(&T::operator())>::Type;
		using FunctionType = typename NativeInvocableCallBuilder<P, decltype(&T::operator())>::FunctionType;
		static Type create(FunctionType &&, LuaCppRuntime &);
	};

	template<typename P, typename R, typename ... A>
	struct NativeInvocableCallBuilder<P, R(A...)> {
		using FunctionType = std::function<R(A...)>;
		using Type = NativeInvocableCall<P, FunctionType, A...>;
	};

	template<typename P, typename R, typename ... A>
	struct NativeInvocableCallBuilder<P, R(*)(A...)> {
		using FunctionType = std::function<R(A...)>;
		using Type = NativeInvocableCall<P, FunctionType, A...>;
	};

	template<typename P, typename C, typename R, typename ... A>
	struct NativeInvocableCallBuilder<P, R (C::*)(A...) const> {
		using FunctionType = std::function<R(A...)>;
		using Type = NativeInvocableCall<P, FunctionType, A...>;
	};

	template <typename P>
	class NativeInvocable {
		template <typename R, typename ... A>
		using FType = R(*)(A...);
	 public:
		template <typename R, typename ... A>
		static NativeFunctionCall<P, R, A...> create(FType<R, A...>, LuaCppRuntime &);

	 	template <typename F>
		static typename NativeInvocableCallBuilder<P, F>::Type create(F, LuaCppRuntime &);
	};
}

#include "luacppb/Invoke/Impl/Native.h"

#endif
