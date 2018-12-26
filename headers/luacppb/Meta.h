#ifndef LUACPPB_META_H_
#define LUACPPB_META_H_

#include "luacppb/Base.h"
#include <type_traits>
#include <memory>

namespace LuaCppB {

	template<typename T>
	struct always_false : std::false_type {};

	template<template<typename...> class, typename...>
	struct is_instantiation : public std::false_type {};

	template<template<typename...> class U, typename... T>
	struct is_instantiation<U, U<T...>> : public std::true_type {};

	template <typename T>
	struct is_smart_pointer {
		static constexpr bool value = is_instantiation<std::unique_ptr, T>::value || is_instantiation<std::shared_ptr, T>::value;
	};

	template <typename T, typename E = void>
	struct is_callable : std::is_function<T> {};

	template <typename T>
	struct is_callable<T, typename std::enable_if<std::is_pointer<T>::value>::type> : std::is_function<typename std::remove_pointer<T>::type> {};

	template <typename T>
	struct is_callable<T, typename std::enable_if<std::is_same<decltype(void(&T::operator())), void>::value>::type> : std::true_type {};
}

#endif