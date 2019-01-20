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

#ifndef LUACPPB_OBJECT_IMPL_BOXER_H_
#define LUACPPB_OBJECT_IMPL_BOXER_H_

#include "luacppb/Object/Boxer.h"

namespace LuaCppB::Internal {

  static std::string EmptyClassName = "";
    
  template <typename T>
  const std::string &LuaCppObjectBoxerRegistry::getClassName() {
    std::type_index idx = std::type_index(typeid(T));
    if (this->wrappers.count(idx)) {
      return this->wrappers[idx]->getClassName();
    } else {
      return EmptyClassName;
    }
  }

  template <typename T>
  void LuaCppObjectBoxerRegistry::copyFields(std::map<std::string, std::shared_ptr<LuaCppObjectFieldPusher>> &fields) {
    std::type_index idx = std::type_index(typeid(T));
    if (this->wrappers.count(idx)) {
      this->wrappers[idx]->copyFields(fields);
    }
  }

  template <typename T>
  bool LuaCppObjectBoxerRegistry::canWrap() {
    return this->wrappers.count(typeid(T)) != 0;
  }

  template <typename T>
  void LuaCppObjectBoxerRegistry::wrap(lua_State *state, T *object) {
    if (this->canWrap<T>()) { // lgtm [cpp/empty-block]
      if constexpr (std::is_const<T>::value) {
        this->wrappers[typeid(T)]->wrap(state, reinterpret_cast<const void *>(object));
      } else {
        this->wrappers[typeid(T)]->wrap(state, reinterpret_cast<void *>(object));
      }
    }
  }

  template <typename T>
  void LuaCppObjectBoxerRegistry::wrap(lua_State *state, std::unique_ptr<T> object) {
    if (this->canWrap<T>()) {
      T *pointer = object.release();
      this->wrappers[typeid(T)]->wrapUnique(state, reinterpret_cast<void *>(pointer));
    }
  }

  template <typename T>
  void LuaCppObjectBoxerRegistry::wrap(lua_State *state, std::shared_ptr<T> object) {
    if (this->canWrap<T>()) {
      this->wrappers[typeid(T)]->wrapShared(state, std::static_pointer_cast<void >(object));
    }
  }

  template <typename T>
  void LuaCppObjectBoxerRegistry::addBoxer(std::shared_ptr<Internal::LuaCppObjectBoxer> wrapper) {
    this->wrappers[typeid(T)] = wrapper;
  }
}

#endif
