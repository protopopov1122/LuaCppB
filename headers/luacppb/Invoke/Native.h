#ifndef LUACPPB_INVOKE_NATIVE_H_
#define LUACPPB_INVOKE_NATIVE_H_

#include "luacppb/Base.h"
#include "luacppb/Value/Native.h"
#include "luacppb/Invoke/Method.h"
#include "luacppb/Core/Runtime.h"
#include "luacppb/Core/State.h"
#include "luacppb/Core/Stack.h"
#include <string>
#include <type_traits>
#include <utility>
#include <tuple>
#include <functional>

namespace LuaCppB {
	
	template <std::size_t I, typename T, typename E = void>
	struct NativeFunctionArgument {
		static T get(lua_State *state) {
			return LuaValue::peek(state, I).value_or(LuaValue()).get<T>();
		}
	};

	template <std::size_t I, typename T>
	struct NativeFunctionArgument<I, T, typename std::enable_if<std::is_same<T, LuaState>::value>::type> {
		static T get(lua_State *state) {
			return LuaState(state);
		}
	};

	template <std::size_t I, typename T>
	struct NativeFunctionArgument<I, T, typename std::enable_if<std::is_same<T, LuaReferenceHandle>::value>::type> {
		static T get(lua_State *state) {
			return LuaState(state)[I];
		}
	};

	template <typename T>
	struct NativeFunctionResult {
		static void set(lua_State *state, LuaCppRuntime &runtime, T &value) {
			LuaNativeValue::push<T>(state, runtime, value);
		}
	};
	
	template <std::size_t I, typename ... Ts>
	struct NativeFunctionArgumentsTuple_Impl {};

	template <std::size_t I>
	struct NativeFunctionArgumentsTuple_Impl<I> {
		static std::tuple<> value(lua_State *state) {
			return std::make_tuple();
		}
	};

	template <std::size_t I, typename T, typename ... Ts>
	struct NativeFunctionArgumentsTuple_Impl<I, T, Ts...> {
		static std::tuple<T, Ts...> value(lua_State *state) {
			return std::tuple_cat(std::make_tuple(NativeFunctionArgument<I, T>::get(state)), NativeFunctionArgumentsTuple_Impl<I + 1, Ts...>::value(state));
		};
	};
	
	template <std::size_t S, typename ... A>
	struct NativeFunctionArgumentsTuple {
		static std::tuple<A...> value(lua_State *state) {
			return NativeFunctionArgumentsTuple_Impl<S, A...>::value(state);
		}
	};

	template <typename R, typename ... A>
	class NativeFunctionCall : public LuaData {
		using F = R (*)(A...);
	 public:
		NativeFunctionCall(F fn, LuaCppRuntime &runtime) : function(fn), runtime(runtime) {}

		void push(lua_State *state) const override {
			lua_pushlightuserdata(state, reinterpret_cast<void *>(this->function));
			lua_pushlightuserdata(state, reinterpret_cast<void *>(&this->runtime));
			lua_pushcclosure(state, NativeFunctionCall<R, A...>::function_closure, 2);
		}
	 private:
		static int call(F function, LuaCppRuntime &runtime, lua_State *state) {
			std::tuple<A...> args = NativeFunctionArgumentsTuple<1, A...>::value(state);
			if constexpr (std::is_void<R>::value) {
				std::apply(function, args);
				return 0;
			} else {				
				R result = std::apply(function, args);
				NativeFunctionResult<R>::set(state, runtime, result);
				return 1;
			}
		};

		static int function_closure(lua_State *state) {
			LuaStack stack(state);
			F fn = stack.toPointer<F>(lua_upvalueindex(1));
			LuaCppRuntime &runtime = *stack.toPointer<LuaCppRuntime *>(lua_upvalueindex(2));
			return NativeFunctionCall<R, A...>::call(fn, runtime, state);
		};

		F function;
		LuaCppRuntime &runtime;
	};

	template <typename C, typename M>
	struct NativeMethodDescriptor {
		C *object;
		M method;
	};

	template <typename C, typename R, typename ... A>
	class NativeMethodCall : public LuaData {
		using M = R (C::*)(A...);
		using Mc = R (C::*)(A...) const;
	 public:
		NativeMethodCall(C *obj, M met, LuaCppRuntime &runtime) : object(obj), method(met), runtime(runtime) {}
		NativeMethodCall(C &obj, M met, LuaCppRuntime &runtime) : object(&obj), method(met), runtime(runtime) {}
		NativeMethodCall(const C *obj, Mc met, LuaCppRuntime &runtime)
			: object(const_cast<C *>(obj)), method(NativeMethodWrapper(met).get()), runtime(runtime) {}
		NativeMethodCall(const C &obj, Mc met, LuaCppRuntime &runtime)
			: object(const_cast<C *>(&obj)), method(NativeMethodWrapper(met).get()), runtime(runtime) {}
		
		void push(lua_State *state) const override {
			NativeMethodDescriptor<C, M> *descriptor = reinterpret_cast<NativeMethodDescriptor<C, M> *>(lua_newuserdata(state, sizeof(NativeMethodDescriptor<C, M>)));
			descriptor->object = this->object;
			descriptor->method = this->method;
			lua_pushlightuserdata(state, reinterpret_cast<void *>(&this->runtime));
			lua_pushcclosure(state, &NativeMethodCall<C, R, A...>::method_closure, 2);	
		}
	 private:
	 	static int call(C *object, M method, LuaCppRuntime &runtime, lua_State *state) {
			std::tuple<A...> args = NativeFunctionArgumentsTuple<1,A...>::value(state);
			if constexpr (std::is_void<R>::value) {
				std::apply([object, method](A... args) {	
					return (object->*method)(args...);
				}, args);
				return 0;
			} else {
				R result = std::apply([object, method](A... args) {	
					return (object->*method)(args...);
				}, args);
				NativeFunctionResult<R>::set(state, runtime, result);
				return 1;
			}
		};

		static int method_closure(lua_State *state) {
			LuaStack stack(state);
			NativeMethodDescriptor<C, M> *descriptor = stack.toUserData<NativeMethodDescriptor<C, M> *>(lua_upvalueindex(1));
			LuaCppRuntime &runtime = *stack.toPointer<LuaCppRuntime *>(lua_upvalueindex(2));
			return NativeMethodCall<C, R, A...>::call(descriptor->object, descriptor->method, runtime, state);
		};

		C *object;
		M method;
		LuaCppRuntime &runtime;
	};

	template <typename T>
	class NativeInvocableDescriptor {
	 public:
	 	NativeInvocableDescriptor(T &value) :invocable(value) {}
		T invocable;
	};

	template <typename T, typename ... A>
	class NativeInvocableCall : public LuaData {
		using R = typename std::invoke_result<T, A...>::type;
	 public:
	 	NativeInvocableCall(T inv, LuaCppRuntime &runtime) : invocable(inv), runtime(runtime) {}

		void push(lua_State *state) const {
			NativeInvocableDescriptor<T> *descriptor = reinterpret_cast<NativeInvocableDescriptor<T> *>(lua_newuserdata(state, sizeof(NativeInvocableDescriptor<T>)));
			new(descriptor) NativeInvocableDescriptor(this->invocable);
			lua_pushlightuserdata(state, reinterpret_cast<void *>(&this->runtime));
			lua_pushcclosure(state, &NativeInvocableCall<T, A...>::invocable_closure, 2);
		}
	 private:
		static int call(T &invocable, LuaCppRuntime &runtime, lua_State *state) {
			std::tuple<A...> args = NativeFunctionArgumentsTuple<1, A...>::value(state);
			if constexpr (std::is_void<R>::value) {
				std::apply(invocable, args);
				return 0;
			} else {
				R result = std::apply(invocable, args);
				NativeFunctionResult<R>::set(state, runtime, result);
				return 1;
			}
		}

		static int invocable_closure(lua_State *state) {
			LuaStack stack(state);
			NativeInvocableDescriptor<T> *descriptor = stack.toUserData<NativeInvocableDescriptor<T> *>(lua_upvalueindex(1));
			LuaCppRuntime &runtime = *stack.toPointer<LuaCppRuntime *>(lua_upvalueindex(2));
			return NativeInvocableCall<T, A...>::call(descriptor->invocable, runtime, state);
		}

		T invocable;
		LuaCppRuntime &runtime;
	};
}

#endif
