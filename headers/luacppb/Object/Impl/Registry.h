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

#ifndef LUACPPB_OBJECT_IMPL_REGISTRY_H_
#define LUACPPB_OBJECT_IMPL_REGISTRY_H_

#include "luacppb/Object/Registry.h"


namespace LuaCppB::Internal {

  template <typename T, typename P>
  LuaCppClassObjectBoxer<T, P>::LuaCppClassObjectBoxer(const std::string &className, std::map<std::string, std::shared_ptr<LuaCppObjectFieldPusher>> &fields)
    : className(className), fields(fields) {}

  template <typename T, typename P>
  void LuaCppClassObjectBoxer<T, P>::wrap(lua_State *state, void *raw_ptr) {
    Internal::LuaStack stack(state);
    T *object = reinterpret_cast<T *>(raw_ptr);
    LuaCppObjectWrapper<T> *wrapper = stack.push<LuaCppObjectWrapper<T>>();
    new(wrapper) LuaCppObjectWrapper<T>(object);
    if constexpr (!std::is_void<P>::value) {
      wrapper->addParentType(std::type_index(typeid(P)));
    }
    stack.setMetatable(this->className);
  }

  template <typename T, typename P>
  void LuaCppClassObjectBoxer<T, P>::wrap(lua_State *state, const void *raw_ptr) {
    Internal::LuaStack stack(state);
    const T *object = reinterpret_cast<const T *>(raw_ptr);
    LuaCppObjectWrapper<const T> *wrapper = stack.push<LuaCppObjectWrapper<const T>>();
    new(wrapper) LuaCppObjectWrapper<const T>(object);
    if constexpr (!std::is_void<P>::value) {
      wrapper->addParentType(std::type_index(typeid(P)));
    }
    stack.setMetatable(this->className);
  }

  template <typename T, typename P>
  void LuaCppClassObjectBoxer<T, P>::wrapUnique(lua_State *state, void *raw_ptr) {
    Internal::LuaStack stack(state);
    std::unique_ptr<T> object = std::unique_ptr<T>(reinterpret_cast<T *>(raw_ptr));
    LuaCppObjectWrapper<T> *wrapper = stack.push<LuaCppObjectWrapper<T>>();
    new(wrapper) LuaCppObjectWrapper<T>(std::move(object));
    if constexpr (!std::is_void<P>::value) {
      wrapper->addParentType(std::type_index(typeid(P)));
    }
    stack.setMetatable(this->className);
  }

  template <typename T, typename P>
  void LuaCppClassObjectBoxer<T, P>::wrapShared(lua_State *state, std::shared_ptr<void> raw_object) {
    Internal::LuaStack stack(state);
    std::shared_ptr<T> object = std::static_pointer_cast<T>(raw_object);
    LuaCppObjectWrapper<T> *wrapper = stack.push<LuaCppObjectWrapper<T>>();
    new(wrapper) LuaCppObjectWrapper<T>(object);
    if constexpr (!std::is_void<P>::value) {
      wrapper->addParentType(std::type_index(typeid(P)));
    }
    stack.setMetatable(this->className);
  }

  template <typename T, typename P>
  std::string LuaCppClassObjectBoxer<T, P>::getClassName() {
    return this->className;
  }

  template <typename T, typename P>
  void LuaCppClassObjectBoxer<T, P>::copyFields(std::map<std::string, std::shared_ptr<LuaCppObjectFieldPusher>> &fields) {
    fields.insert(this->fields.begin(), this->fields.end());
  }

  template <typename T, typename P>
  void LuaCppClassRegistry::bind(LuaCppClass<T, P> &cl) {
    cl.bind(this->state);
    std::map<std::string, std::shared_ptr<LuaCppObjectFieldPusher>> map;
    cl.copyFields(map);
    this->addBoxer<T>(std::make_shared<LuaCppClassObjectBoxer<T, P>>(cl.getClassName(), map));
  }
}

#endif
