#ifndef LUACPPB_OBJECT_WRAPPER_H_
#define LUACPPB_OBJECT_WRAPPER_H_

#include "luacppb/Base.h"
#include "luacppb/Core/Error.h"
#include <memory>
#include <variant>
#include <typeinfo>
#include <typeindex>
#include <type_traits>
#include <set>

namespace LuaCppB {

	template <typename C>
	class LuaCppObjectWrapper {
		using Raw = C *;
		using Unique = std::unique_ptr<C>;
		using Shared = std::shared_ptr<C>;
	 public:
	 	LuaCppObjectWrapper();
	 	LuaCppObjectWrapper(Raw);
		LuaCppObjectWrapper(C &);
		LuaCppObjectWrapper(Unique);
		LuaCppObjectWrapper(Shared);
		~LuaCppObjectWrapper();

		void addParentType(std::type_index);
		C *get();
	 private:
		std::variant<Raw, Unique, Shared> object;
		std::set<std::type_index> objectType;
		bool constant;
	};
}

#include "luacppb/Object/Impl/Wrapper.h"

#endif