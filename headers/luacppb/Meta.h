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
}

#endif