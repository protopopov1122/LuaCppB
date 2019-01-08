#ifndef LUACPPB_OBJECT_IMPL_WRAPPER_H_
#define LUACPPB_OBJECT_IMPL_WRAPPER_H_

#include "luacppb/Object/Wrapper.h"

namespace LuaCppB {

	template <typename C>
	LuaCppObjectWrapper<C>::LuaCppObjectWrapper(Raw obj)
    : object(obj), objectType({ typeid(C) }), constant(std::is_const<C>::value) {}

	template <typename C>
	LuaCppObjectWrapper<C>::LuaCppObjectWrapper(C &obj)
    : object(&obj), objectType({ typeid(C) }), constant(std::is_const<C>::value) {}

	template <typename C>
	LuaCppObjectWrapper<C>::LuaCppObjectWrapper() : objectType({ typeid(C) }), constant(std::is_const<C>::value) {
    this->object = std::unique_ptr<C>(reinterpret_cast<C *>(::operator new(sizeof(C))));
  }

	template <typename C>
  LuaCppObjectWrapper<C>::LuaCppObjectWrapper(Unique obj)
    : object(std::move(obj)), objectType({ typeid(C) }), constant(std::is_const<C>::value) {}

	template <typename C>
  LuaCppObjectWrapper<C>::LuaCppObjectWrapper(Shared obj)
    : object(obj), objectType({ typeid(C) }), constant(std::is_const<C>::value) {}
	
	template <typename C>
  LuaCppObjectWrapper<C>::~LuaCppObjectWrapper() {
    this->object = nullptr;
  }

	template <typename C>
  void LuaCppObjectWrapper<C>::addParentType(std::type_index idx) {
    this->objectType.insert(idx);
  }

	template <typename C>
  C *LuaCppObjectWrapper<C>::get() {
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
}

#endif