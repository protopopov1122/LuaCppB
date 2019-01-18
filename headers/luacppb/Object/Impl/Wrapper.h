/*
  SPDX short identifier: MIT

  Copyright 2018-2019 Jevgēnijs Protopopovs

  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"),
  to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
  and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
  IN THE SOFTWARE.
*/

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
