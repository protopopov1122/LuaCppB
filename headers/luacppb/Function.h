#ifndef LUACPPB_FUNCTION_H_
#define LUACPPB_FUNCTION_H_

#include "luacppb/Base.h"
#include "luacppb/Value.h"
#include <string>
#include <type_traits>
#include <utility>
#include <tuple>

namespace LuaCppB {
	
	template <std::size_t I, typename T, typename E = void>
	struct CFunctionArgument {};

	template <std::size_t I, typename T>
	struct CFunctionArgument<I, T, typename std::enable_if<std::is_integral<T>::value && !std::is_same<T, bool>::value>::type> {
		static T get(lua_State *state) {
			lua_Integer value = lua_tointeger(state, I);			
			return static_cast<T>(value);
		}

	};

	template <std::size_t I, typename T>
	struct CFunctionArgument<I, T, typename std::enable_if<std::is_floating_point<T>::value>::type> {

		static T get(lua_State *state) {
			lua_Number value = lua_tonumber(state, I);
			return static_cast<T>(value);
		}
	};

	template <std::size_t I, typename T>
	struct CFunctionArgument<I, T, typename std::enable_if<std::is_same<T, bool>::value>::type> {

		static T get(lua_State *state) {
			int value = lua_toboolean(state, I);
			return static_cast<T>(value);
		}
	};

	template <std::size_t I, typename T>
	struct CFunctionArgument<I, T, typename std::enable_if<std::is_same<T, std::string>::value>::type> {

		static T get(lua_State *state) {
			const char *value = lua_tostring(state, I);
			return std::string(value);
		}
	};

	template <std::size_t I, typename T>
	struct CFunctionArgument<I, T, typename std::enable_if<std::is_same<T, LuaState>::value>::type> {
		
		static T get(lua_State *state) {
			return LuaState(state);
		}
	};

	template <std::size_t I, typename T>
	struct CFunctionArgument<I, T, typename std::enable_if<std::is_same<T, LuaReferenceHandle>::value>::type> {
		
		static T get(lua_State *state) {
			return LuaState(state)[I];
		}
	};


	template <typename T>
	struct CFunctionResult {
		static void set(lua_State *state, T value) {
			LuaValue::create<T>(value).push(state);
		}
	};
	
	template <std::size_t I, typename ... Ts>
	struct FunctionArgumentsTuple_Impl {};

	template <std::size_t I>
	struct FunctionArgumentsTuple_Impl<I> {
		static std::tuple<> value(lua_State *state) {
			return std::make_tuple();
		}
	};

	template <std::size_t I, typename T, typename ... Ts>
	struct FunctionArgumentsTuple_Impl<I, T, Ts...> {
		static std::tuple<T, Ts...> value(lua_State *state) {
			return std::tuple_cat(std::make_tuple(CFunctionArgument<I, T>::get(state)), FunctionArgumentsTuple_Impl<I + 1, Ts...>::value(state));
		};
	};

	
	template <typename ... A>
	struct FunctionArgumentsTuple {
		static std::tuple<A...> value(lua_State *state) {
			return FunctionArgumentsTuple_Impl<1, A...>::value(state);
		}
	};


	template <typename R, typename ... A>
	class CFunctionCall : public LuaPushable {
		using F = R (*)(A...);
	 public:
		CFunctionCall(F fn) : function(fn) {}

		void push(lua_State *state) const override {
			lua_pushlightuserdata(state, reinterpret_cast<void *>(this->function));
			lua_pushcclosure(state, CFunctionCall<R, A...>::function_closure, 1);
		}

		static int call(F function, lua_State *state) {
			std::tuple<A...> args = FunctionArgumentsTuple<A...>::value(state);
			if constexpr (std::is_void<R>::value) {
				std::apply(function, args);
				return 0;
			} else {				
				R result = std::apply(function, args);
				CFunctionResult<R>::set(state, result);
				return 1;
			}
		};
	 private:
		static int function_closure(lua_State *state) {
			const void *fn = lua_topointer(state, lua_upvalueindex(1));
			return CFunctionCall<R, A...>::call(reinterpret_cast<F>(fn), state);
		};

		F function;
	};

	template <typename C, typename M>
	struct CMethodDescriptor {
		C *object;
		M method;
	};

	template <typename C, typename R, typename ... A>
	class CMethodCall : public LuaPushable {
		using M = R (C::*)(A...);
	 public:
		CMethodCall(C *obj, M met) : object(obj), method(met) {}
		void push(lua_State *state) const override {
			CMethodDescriptor<C, M> *descriptor = reinterpret_cast<CMethodDescriptor<C, M> *>(lua_newuserdata(state, sizeof(CMethodDescriptor<C, M>)));
			descriptor->object = this->object;
			descriptor->method = this->method;
			lua_pushcclosure(state, &CMethodCall<C, R, A...>::method_closure, 1);	
		}


		static int call(C *object, M method, lua_State *state) {
			std::tuple<A...> args = FunctionArgumentsTuple<A...>::value(state);
			if constexpr (std::is_void<R>::value) {
				return 0;
			} else {
				R result = std::apply([object, method](A... args) {	
					return (object->*method)(args...);
				}, args);
				CFunctionResult<R>::set(state, result);
				return 1;
			}
		};
	 private:
		static int method_closure(lua_State *state) {
			CMethodDescriptor<C, M> *descriptor = reinterpret_cast<CMethodDescriptor<C, M> *>(lua_touserdata(state, lua_upvalueindex(1)));
			return CMethodCall<C, R, A...>::call(descriptor->object, descriptor->method, state);
		};

		C *object;
		M method;
	};
}

#endif
