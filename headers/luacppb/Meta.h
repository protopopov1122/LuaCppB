#ifndef LUACPPB_META_H_
#define LUACPPB_META_H_

#include "luacppb/Base.h"
#include <type_traits>

namespace LuaCppB {

	template<typename T>
	struct always_false : std::false_type {};
}

#endif