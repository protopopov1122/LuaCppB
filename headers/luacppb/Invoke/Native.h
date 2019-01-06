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
		NativeFunctionCall(F fn, LuaCppRuntime &runtime) : function(fn), runtime(runtime) {}

		void push(lua_State *state) const override {
			Internal::LuaStack stack(state);
			stack.push(reinterpret_cast<void*>(this->function));
			stack.push(&this->runtime);
			stack.push(&NativeFunctionCall<P, R, A...>::function_closure, 2);
		}
	 private:
		static int call(F function, LuaCppRuntime &runtime, lua_State *state) {
			std::array<LuaValue, sizeof...(A)> wrappedArgs;
			WrappedFunctionArguments<1, A...>::get(state, wrappedArgs);
			std::tuple<A...> args = Internal::NativeFunctionArgumentsTuple<1, A...>::value(state, runtime, wrappedArgs);
			if constexpr (std::is_void<R>::value) {
				std::apply(function, args);
				return 0;
			} else {
				return Internal::NativeFunctionResult<P, R>::set(state, runtime, std::apply(function, args));
			}
		};

		static int function_closure(lua_State *state) {
			try {
				Internal::LuaStack stack(state);
				F fn = stack.toPointer<F>(lua_upvalueindex(1));
				LuaCppRuntime &runtime = *stack.toPointer<LuaCppRuntime *>(lua_upvalueindex(2));
				return NativeFunctionCall<P, R, A...>::call(fn, runtime, state);
			} catch (std::exception &ex) {
				return luacpp_handle_exception(state, std::current_exception());
			}
		};

		F function;
		LuaCppRuntime &runtime;
	};

	template <typename P, typename C, typename R, typename ... A>
	class NativeMethodCall : public LuaData {
		using M = R (C::*)(A...);
		using Mc = R (C::*)(A...) const;
	 public:
		NativeMethodCall(C *obj, M met, LuaCppRuntime &runtime) : object(obj), method(met), runtime(runtime) {}
		NativeMethodCall(C &obj, M met, LuaCppRuntime &runtime) : object(&obj), method(met), runtime(runtime) {}
		NativeMethodCall(const C *obj, Mc met, LuaCppRuntime &runtime)
			: object(const_cast<C *>(obj)), method(Internal::NativeMethodWrapper<C, R, A...>(met).get()), runtime(runtime) {}
		NativeMethodCall(const C &obj, Mc met, LuaCppRuntime &runtime)
			: object(const_cast<C *>(&obj)), method(Internal::NativeMethodWrapper<C, R, A...>(met).get()), runtime(runtime) {}
		
		void push(lua_State *state) const override {
			Internal::LuaStack stack(state);
			NativeMethodDescriptor<C, M> *descriptor = NativeMethodDescriptor<C, M>::pushDescriptor(state);
			descriptor->object = this->object;
			descriptor->method = this->method;
			stack.push(&this->runtime);
			stack.push(&NativeMethodCall<P, C, R, A...>::method_closure, 2);
		}
	 private:
	 	static int call(C *object, M &&method, LuaCppRuntime &runtime, lua_State *state) {
			std::array<LuaValue, sizeof...(A)> wrappedArgs;
			WrappedFunctionArguments<1, A...>::get(state, wrappedArgs);
			std::tuple<A...> args = Internal::NativeFunctionArgumentsTuple<1, A...>::value(state, runtime, wrappedArgs);
			if constexpr (std::is_void<R>::value) {
				std::apply([object, method](A... args) {	
					return (object->*method)(args...);
				}, args);
				return 0;
			} else {
				return Internal::NativeFunctionResult<P, R>::set(state, runtime, std::apply([object, method](A... args)->R {	
					return (object->*method)(args...);
				}, args));
			}
		};

		static int method_closure(lua_State *state) {
			try {
				Internal::LuaStack stack(state);
				NativeMethodDescriptor<C, M> *descriptor = stack.toUserData<NativeMethodDescriptor<C, M> *>(lua_upvalueindex(1));
				LuaCppRuntime &runtime = *stack.toPointer<LuaCppRuntime *>(lua_upvalueindex(2));
				return NativeMethodCall<P, C, R, A...>::call(descriptor->object, std::forward<M>(descriptor->method), runtime, state);
			} catch (std::exception &ex) {
				return luacpp_handle_exception(state, std::current_exception());
			}
		};

		C *object;
		M method;
		LuaCppRuntime &runtime;
	};

	template <typename P>
	class NativeMethod {
	 public:
	 	template <typename C, typename R, typename ... A>
	 	static NativeMethodCall<P, C, R, A...> create(C &obj, R (C::*met)(A...), LuaCppRuntime &runtime) {
			return NativeMethodCall<P, C, R, A...>(obj, met, runtime);
		}
		
	 	template <typename C, typename R, typename ... A>
	 	static NativeMethodCall<P, C, R, A...> create(C *obj, R (C::*met)(A...), LuaCppRuntime &runtime) {
			return NativeMethodCall<P, C, R, A...>(obj, met, runtime);
		}

	 	template <typename C, typename R, typename ... A>
	 	static NativeMethodCall<P, C, R, A...> create(C &obj, R (C::*met)(A...) const, LuaCppRuntime &runtime) {
			return NativeMethodCall<P, C, R, A...>(obj, met, runtime);
		}
		
	 	template <typename C, typename R, typename ... A>
	 	static NativeMethodCall<P, C, R, A...> create(C *obj, R (C::*met)(A...) const, LuaCppRuntime &runtime) {
			return NativeMethodCall<P, C, R, A...>(obj, met, runtime);
		}

	 	template <typename C, typename R, typename ... A>
	 	static NativeMethodCall<P, C, R, A...> create(const C &obj, R (C::*met)(A...) const, LuaCppRuntime &runtime) {
			return NativeMethodCall<P, C, R, A...>(obj, met, runtime);
		}
		
	 	template <typename C, typename R, typename ... A>
	 	static NativeMethodCall<P, C, R, A...> create(const C *obj, R (C::*met)(A...) const, LuaCppRuntime &runtime) {
			return NativeMethodCall<P, C, R, A...>(obj, met, runtime);
		}
	};

	template <typename P, typename T, typename ... A>
	class NativeInvocableCall : public LuaData {
		using R = typename std::invoke_result<T, A...>::type;
	 public:
	 	NativeInvocableCall(T &&inv, LuaCppRuntime &runtime) : invocable(inv), runtime(runtime) {}

		void push(lua_State *state) const {
			Internal::LuaStack stack(state);
			NativeInvocableDescriptor<T> *descriptor = NativeInvocableDescriptor<T>::pushDescriptor(state);
			new(descriptor) NativeInvocableDescriptor(this->invocable);
			stack.push(&this->runtime);
			stack.push(&NativeInvocableCall<P, T, A...>::invocable_closure, 2);
		}
	 private:
		static int call(T &invocable, LuaCppRuntime &runtime, lua_State *state) {
			std::array<LuaValue, sizeof...(A)> wrappedArgs;
			WrappedFunctionArguments<1, A...>::get(state, wrappedArgs);
			std::tuple<A...> args = Internal::NativeFunctionArgumentsTuple<1, A...>::value(state, runtime, wrappedArgs);
			if constexpr (std::is_void<R>::value) {
				std::apply(invocable, args);
				return 0;
			} else {
				return Internal::NativeFunctionResult<P, R>::set(state, runtime, std::apply(invocable, args));
			}
		}

		static int invocable_closure(lua_State *state) {
			try {
				Internal::LuaStack stack(state);
				NativeInvocableDescriptor<T> *descriptor = stack.toUserData<NativeInvocableDescriptor<T> *>(lua_upvalueindex(1));
				LuaCppRuntime &runtime = *stack.toPointer<LuaCppRuntime *>(lua_upvalueindex(2));
				return NativeInvocableCall<P, T, A...>::call(descriptor->invocable, runtime, state);
			} catch (std::exception &ex) {
				return luacpp_handle_exception(state, std::current_exception());
			}
		}

		T invocable;
		LuaCppRuntime &runtime;
	};

	template<typename P, typename T>
	struct NativeInvocableCallBuilder : public NativeInvocableCallBuilder<P, decltype(&T::operator())> {
		using Type = typename NativeInvocableCallBuilder<P, decltype(&T::operator())>::Type;
		using FunctionType = typename NativeInvocableCallBuilder<P, decltype(&T::operator())>::FunctionType;
		static Type create(FunctionType && f, LuaCppRuntime &runtime) {
			return Type(std::forward<FunctionType>(f), runtime);
		}
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
		static NativeFunctionCall<P, R, A...> create(FType<R, A...> func, LuaCppRuntime &runtime) {
			return NativeFunctionCall<P, R, A...>(std::forward<FType<R, A...>>(func), runtime);
		}

	 	template <typename F>
		static typename NativeInvocableCallBuilder<P, F>::Type create(F func, LuaCppRuntime &runtime) {
			return NativeInvocableCallBuilder<P, F>::create(std::forward<F>(func), runtime);
		}
	};
}

#endif
