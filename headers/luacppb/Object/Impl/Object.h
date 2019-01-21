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

#ifndef LUACPPB_OBJECT_IMPL_OBJECT_H_
#define LUACPPB_OBJECT_IMPL_OBJECT_H_

#include "luacppb/Object/Object.h"

namespace LuaCppB {

  template <typename T>
  LuaCppObject<T>::LuaCppObject(T *obj, LuaCppRuntime &runtime) : object(obj), runtime(runtime) {
    std::stringstream ss;
    ss << "$object" << (LuaCppObject::nextIdentifier++) << '$' << typeid(T).name() << '@' << this->object;
    this->className = ss.str();
  }

  template <typename T>
  LuaCppObject<T>::LuaCppObject(T &obj, LuaCppRuntime &runtime) : LuaCppObject(&obj, runtime) {}

  template <typename T>
  template <typename R, typename ... A>
  void LuaCppObject<T>::bind(const std::string &key, R (T::*method)(A...)) {
    using M = R (T::*)(A...);
    this->methods[key] = std::make_shared<Internal::LuaCppObjectMethodCall<T, M, R, A...>>(method, this->className, this->runtime);
  }

  template <typename T>
  template <typename R, typename ... A>
  void LuaCppObject<T>::bind(const std::string &key, R (T::*method)(A...) const) {
    using M = R (T::*)(A...) const;
    this->methods[key] = std::make_shared<Internal::LuaCppObjectMethodCall<const T, M, R, A...>>(method, this->className, this->runtime);
  }

  template <typename T>
  template <typename V, typename B>
  typename std::enable_if<std::is_base_of<B, T>::value>::type
    LuaCppObject<T>::bind(const std::string &key, V B::*field) {
    this->fields[key] = std::make_shared<Internal::LuaCppObjectFieldHandle<B, T, V>>(field, this->runtime);
  }

  template <typename T>
  template <typename V>
  typename std::enable_if<Internal::LuaValueWrapper<V>::Conversible>::type
    LuaCppObject<T>::bind(const std::string &key, V &value) {
    this->dataFields[key] = Internal::LuaValueWrapper<V>::wrap(value);
  }

  template <typename T>
  template <typename V>
  typename std::enable_if<Internal::LuaValueWrapper<V>::Conversible>::type
    LuaCppObject<T>::bind(const std::string &key, V &&value) {
    this->dataFields[key] = Internal::LuaValueWrapper<V>::wrap(std::forward<V>(value));
  }

  template <typename T>
  void LuaCppObject<T>::push(lua_State *state) const {
    Internal::LuaStack stack(state);
    LuaCppObjectWrapper<T> *object = stack.push<LuaCppObjectWrapper<T>>();
    new(object) LuaCppObjectWrapper<T>(this->object);
    if (stack.metatable(this->className)) {
      stack.pushTable();
      for (auto it = this->methods.begin(); it != this->methods.end(); ++it) {
        it->second->push(state);
        stack.setField(-2, it->first);
      }

      for (auto it = this->dataFields.begin(); it != this->dataFields.end(); ++it) {
        it->second.push(state);
        stack.setField(-2, it->first);
      }
      
      Internal::LuaCppObjectFieldController::pushFunction(state, this->fields);

      stack.push(&LuaCppObject<T>::lookupObject, 2);
      stack.setField(-2, "__index");
      stack.push(&LuaCppObject<T>::gcObject);
      stack.setField(-2, "__gc");
      stack.pushTable();
      stack.setField(-2, "__metatable");
    }
    stack.setMetatable(-2);
  }

  template <typename T>
  int LuaCppObject<T>::lookupObject(lua_State *state) {
    Internal::LuaStack stack(state);
    
    stack.copy(lua_upvalueindex(1));
    stack.copy(2);
    stack.getField(-2);
    stack.remove(-2);
    
    if (stack.isNoneOrNil(-1)) {
      stack.pop(1);
      stack.copy(lua_upvalueindex(2));
      stack.copy(1);
      stack.copy(2);
      lua_pcall(state, 2, 1, 0);
    }
    return 1;
  }
  
  template <typename T>
  int LuaCppObject<T>::gcObject(lua_State *state) {
    Internal::LuaStack stack(state);
    LuaCppObjectWrapper<T> *object = stack.toUserData<LuaCppObjectWrapper<T> *>(1);
    object->~LuaCppObjectWrapper();
    ::operator delete(object, object);
    return 0;
  }

  template <typename T>
  std::atomic<uint64_t> LuaCppObject<T>::nextIdentifier(0);
}

#endif
