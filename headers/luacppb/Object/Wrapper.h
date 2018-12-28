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
	 	LuaCppObjectWrapper(Raw obj) : object(obj), objectType({ typeid(C) }), constant(std::is_const<C>::value) {}
		LuaCppObjectWrapper(C &obj) : object(&obj), objectType({ typeid(C) }), constant(std::is_const<C>::value) {}
	 	LuaCppObjectWrapper() : objectType({ typeid(C) }), constant(std::is_const<C>::value) {
		  this->object = std::unique_ptr<C>(reinterpret_cast<C *>(::operator new(sizeof(C))));
		}
		LuaCppObjectWrapper(Unique obj) : object(std::move(obj)), objectType({ typeid(C) }), constant(std::is_const<C>::value) {}
		LuaCppObjectWrapper(Shared obj) : object(obj), objectType({ typeid(C) }), constant(std::is_const<C>::value) {}
		~LuaCppObjectWrapper() {
			this->object = nullptr;
		}

		void addParentType(std::type_index idx) {
			this->objectType.insert(idx);
		}

		C *get() {
			if (this->objectType.count(std::type_index(typeid(C))) == 0 ||
				(!std::is_const<C>::value && this->constant)) {
				throw LuaCppBError("Type mismatch", LuaCppBErrorCode::IncorrectTypeCast);
			}
			switch (this->object.index()) {
				case 0:
				  return std::get<Raw>(object);
				case 1:
					return std::get<Unique>(object).get();
				case 2:
					return std::get<Shared>(object).get();
				default:
					return nullptr;
			}
		}
	 private:
		std::variant<Raw, Unique, Shared> object;
		std::set<std::type_index> objectType;
		bool constant;
	};
}

#endif