#ifndef LUACPPB_REFERENCE_BASE_H_
#define LUACPPB_REFERENCE_BASE_H_

#include "luacppb/Base.h"
#include "luacppb/Value/Value.h"
#include "luacppb/Value/Native.h"
#include "luacppb/Invoke/Coro.h"
#include "luacppb/Core/Runtime.h"
#include <optional>
#include <type_traits>
#include <functional>

namespace LuaCppB::Internal {

#ifdef LUACPPB_COROUTINE_SUPPORT
	template <typename T>
	struct LuaReferenceGetSpecialCase {
		static constexpr bool value = LuaValueGetSpecialCase<T>::value ||
		                              std::is_same<T, LuaCoroutine>::value;
	};
#else
	template <typename T>
	using LuaReferenceGetSpecialCase = LuaValueGetSpecialCase<T>;
#endif

	class LuaReference {
	 public:
	 	LuaReference(LuaCppRuntime &);
		virtual ~LuaReference() = default;
		LuaCppRuntime &getRuntime();

		virtual bool putOnTop(std::function<void (lua_State *)>) = 0;
		virtual bool setValue(std::function<void (lua_State *)>) = 0;

		template <typename T>
		typename std::enable_if<std::is_same<T, LuaValue>::value, T>::type get();
#ifdef LUACPPB_COROUTINE_SUPPORT
		template <typename T>
		typename std::enable_if<std::is_convertible<LuaValue, T>::value && !std::is_same<T, LuaValue>::value && !std::is_same<T, LuaCoroutine>::value, T>::type
			get();

		template <typename T>
		typename std::enable_if<std::is_same<T, LuaCoroutine>::value, T>::type
			get();
#else
		template <typename T>
		typename std::enable_if<std::is_convertible<LuaValue, T>::value && !std::is_same<T, LuaValue>::value, T>::type
			get();
#endif

		template <typename T>
		typename std::enable_if<std::is_base_of<LuaData, T>::value>::type set(T &);

		template <typename T>
		typename std::enable_if<!std::is_base_of<LuaData, T>::value>::type set(T &);
	 protected:
		LuaCppRuntime &runtime;
	};
}

#include "luacppb/Reference/Impl/Base.h"

#endif