#ifndef LUACPPB_INVOKE_NATIVE_H_
#define LUACPPB_INVOKE_NATIVE_H_

#include "luacppb/Base.h"
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
		static T get(lua_State *state, LuaCppRuntime &runtime) {
			return LuaValue::peek(state, I).value_or(LuaValue()).get<T>();
		}

		static constexpr bool Virtual = false;
	};

	template <std::size_t I, typename T>
	struct NativeFunctionArgument<I, T, typename std::enable_if<std::is_same<T, LuaState>::value>::type> {
		static T get(lua_State *state, LuaCppRuntime &runtime) {
			std::shared_ptr<LuaCppObjectBoxerRegistry> reg = runtime.getOwnedObjectBoxerRegistry();
			std::shared_ptr<LuaCppClassRegistry> clReg = std::static_pointer_cast<LuaCppClassRegistry>(reg);
			return LuaState(state, clReg);
		}

		static constexpr bool Virtual = true;
	};

	template <std::size_t I, typename T>
	struct NativeFunctionArgument<I, T, typename std::enable_if<std::is_same<T, LuaReferenceHandle>::value>::type> {
		static T get(lua_State *state, LuaCppRuntime &runtime) {
			std::shared_ptr<LuaCppObjectBoxerRegistry> reg = runtime.getOwnedObjectBoxerRegistry();
			std::shared_ptr<LuaCppClassRegistry> clReg = std::static_pointer_cast<LuaCppClassRegistry>(reg);
			return LuaState(state, clReg)[I];
		}

		static constexpr bool Virtual = false;
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
			LuaStack stack(state);
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

	template <typename P, typename R, typename ... A>
	class NativeFunctionCall : public LuaData {
		using F = R (*)(A...);
	 public:
		NativeFunctionCall(F fn, LuaCppRuntime &runtime) : function(fn), runtime(runtime) {}

		void push(lua_State *state) const override {
			LuaStack stack(state);
			stack.push(reinterpret_cast<void*>(this->function));
			stack.push(&this->runtime);
			stack.push(&NativeFunctionCall<P, R, A...>::function_closure, 2);
		}
	 private:
		static int call(F function, LuaCppRuntime &runtime, lua_State *state) {
			std::tuple<A...> args = NativeFunctionArgumentsTuple<1, A...>::value(state, runtime);
			if constexpr (std::is_void<R>::value) {
				std::apply(function, args);
				return 0;
			} else {				
				R result = std::apply(function, args);
				return NativeFunctionResult<P, R>::set(state, runtime, result);
			}
		};

		static int function_closure(lua_State *state) {
			LuaStack stack(state);
			F fn = stack.toPointer<F>(lua_upvalueindex(1));
			LuaCppRuntime &runtime = *stack.toPointer<LuaCppRuntime *>(lua_upvalueindex(2));
			return NativeFunctionCall<P, R, A...>::call(fn, runtime, state);
		};

		F function;
		LuaCppRuntime &runtime;
	};

	template <typename C, typename M>
	struct NativeMethodDescriptor {
		C *object;
		M method;
	};

	template <typename P, typename C, typename R, typename ... A>
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
			LuaStack stack(state);
			NativeMethodDescriptor<C, M> *descriptor = stack.push<NativeMethodDescriptor<C, M>>();
			descriptor->object = this->object;
			descriptor->method = this->method;
			stack.push(&this->runtime);
			stack.push(&NativeMethodCall<P, C, R, A...>::method_closure, 2);
		}
	 private:
	 	static int call(C *object, M method, LuaCppRuntime &runtime, lua_State *state) {
			std::tuple<A...> args = NativeFunctionArgumentsTuple<1,A...>::value(state, runtime);
			if constexpr (std::is_void<R>::value) {
				std::apply([object, method](A... args) {	
					return (object->*method)(args...);
				}, args);
				return 0;
			} else {
				R result = std::apply([object, method](A... args) {	
					return (object->*method)(args...);
				}, args);
				return NativeFunctionResult<P, R>::set(state, runtime, result);
			}
		};

		static int method_closure(lua_State *state) {
			LuaStack stack(state);
			NativeMethodDescriptor<C, M> *descriptor = stack.toUserData<NativeMethodDescriptor<C, M> *>(lua_upvalueindex(1));
			LuaCppRuntime &runtime = *stack.toPointer<LuaCppRuntime *>(lua_upvalueindex(2));
			return NativeMethodCall<P, C, R, A...>::call(descriptor->object, descriptor->method, runtime, state);
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
	 	static NativeMethodCall<P, C, R, A...> create(const C &obj, R (C::*met)(A...) const, LuaCppRuntime &runtime) {
			return NativeMethodCall<P, C, R, A...>(obj, met, runtime);
		}
		
	 	template <typename C, typename R, typename ... A>
	 	static NativeMethodCall<P, C, R, A...> create(const C *obj, R (C::*met)(A...) const, LuaCppRuntime &runtime) {
			return NativeMethodCall<P, C, R, A...>(obj, met, runtime);
		}
	};

	template <typename T>
	class NativeInvocableDescriptor {
	 public:
	 	NativeInvocableDescriptor(T &value) :invocable(value) {}
		T invocable;
	};

	template <typename P, typename T, typename ... A>
	class NativeInvocableCall : public LuaData {
		using R = typename std::invoke_result<T, A...>::type;
	 public:
	 	NativeInvocableCall(T inv, LuaCppRuntime &runtime) : invocable(inv), runtime(runtime) {}

		void push(lua_State *state) const {
			LuaStack stack(state);
			NativeInvocableDescriptor<T> *descriptor = stack.push<NativeInvocableDescriptor<T>>();
			new(descriptor) NativeInvocableDescriptor(this->invocable);
			stack.push(&this->runtime);
			stack.push(&NativeInvocableCall<P, T, A...>::invocable_closure, 2);
		}
	 private:
		static int call(T &invocable, LuaCppRuntime &runtime, lua_State *state) {
			std::tuple<A...> args = NativeFunctionArgumentsTuple<1, A...>::value(state, runtime);
			if constexpr (std::is_void<R>::value) {
				std::apply(invocable, args);
				return 0;
			} else {
				R result = std::apply(invocable, args);
				return NativeFunctionResult<P, R>::set(state, runtime, result);
			}
		}

		static int invocable_closure(lua_State *state) {
			LuaStack stack(state);
			NativeInvocableDescriptor<T> *descriptor = stack.toUserData<NativeInvocableDescriptor<T> *>(lua_upvalueindex(1));
			LuaCppRuntime &runtime = *stack.toPointer<LuaCppRuntime *>(lua_upvalueindex(2));
			return NativeInvocableCall<P, T, A...>::call(descriptor->invocable, runtime, state);
		}

		T invocable;
		LuaCppRuntime &runtime;
	};

	template<typename P, typename T>
	struct NativeInvocableBuilder : public NativeInvocableBuilder<P, decltype(&T::operator())> {};

	template<typename P, typename R, typename ... A>
	struct NativeInvocableBuilder<P, R(A...)> {
		using FunctionType = std::function<R(A...)>;
		using Type = NativeInvocableCall<P, FunctionType, A...>;
		static Type create(FunctionType && f, LuaCppRuntime &runtime) {
			return Type(f, runtime);
		}
	};

	template<typename P, typename R, typename ... A>
	struct NativeInvocableBuilder<P, R(*)(A...)> {
		using FunctionType = std::function<R(A...)>;
		using Type = NativeInvocableCall<P, FunctionType, A...>;
		static Type create(FunctionType && f, LuaCppRuntime &runtime) {
			return Type(f, runtime);
		}
	};

	template<typename P, typename C, typename R, typename ... A>
	struct NativeInvocableBuilder<P, R (C::*)(A...) const> {
		using FunctionType = std::function<R(A...)>;
		using Type = NativeInvocableCall<P, FunctionType, A...>;
		static Type create(FunctionType && f, LuaCppRuntime &runtime) {
			return Type(f, runtime);
		}
	};

	template <typename P>
	class NativeInvocable {
	 public:
	 	template <typename F>
		static typename NativeInvocableBuilder<P, F>::Type create(F && func, LuaCppRuntime &runtime) {
			return NativeInvocableBuilder<P, F>::create(func, runtime);
		}
	};
}

#endif
