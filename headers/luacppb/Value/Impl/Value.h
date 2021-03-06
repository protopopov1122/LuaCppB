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

#ifndef LUACPPB_VALUE_IMPL_VALUE_H_
#define LUACPPB_VALUE_IMPL_VALUE_H_

#include "luacppb/Value/Value.h"

namespace LuaCppB {

  template <typename T>
  typename std::enable_if<std::is_integral<T>::value && !std::is_same<T, bool>::value, T>::type
    LuaValue::get(T defaultValue) const {
    if (this->type == LuaType::Number) {
      assert(this->value.index() == 0 || this->value.index() == 1);
      if (this->value.index() == 0) {
        return static_cast<T>(std::get<LuaInteger>(this->value));
      } else {
        return static_cast<T>(std::get<LuaNumber>(this->value));
      }
    } else {
      return defaultValue;
    }
  }

  template <typename T>
  typename std::enable_if<std::is_floating_point<T>::value, T>::type
    LuaValue::get(T defaultValue) const {
    if (this->type == LuaType::Number) {
      assert(this->value.index() == 0 || this->value.index() == 1);
      if (this->value.index() == 0) {
        return static_cast<T>(std::get<LuaInteger>(this->value));
      } else {
        return static_cast<T>(std::get<LuaNumber>(this->value));
      }
    } else {
      return defaultValue;
    }
  }

  template <typename T>
  typename std::enable_if<std::is_same<T, bool>::value, T>::type
    LuaValue::get(T defaultValue) const {
    if (this->type == LuaType::Boolean) {
      assert(this->value.index() == 2);
      return static_cast<T>(std::get<LuaBoolean>(this->value));
    } else {
      return defaultValue;
    }
  }

  template <typename T>
  typename std::enable_if<std::is_same<T, std::string>::value, T>::type
    LuaValue::get(const T &defaultValue) const {
    if (this->type == LuaType::String) {
      assert(this->value.index() == 3);
      return static_cast<T>(std::get<LuaString>(this->value));
    } else {
      return defaultValue;
    }
  }

  template <typename T>
  typename std::enable_if<std::is_reference<T>::value && std::is_same<typename std::decay<T>::type, std::string>::value, T>::type
    LuaValue::get(const T &defaultValue) const {
    if (this->type == LuaType::String) {
      assert(this->value.index() == 3);
      return static_cast<T>(std::get<LuaString>(this->value));
    } else {
      return defaultValue;
    }
  }

  template <typename T>
  typename std::enable_if<std::is_same<T, LuaCFunction>::value, T>::type
    LuaValue::get(T defaultValue) const {
    if (this->type == LuaType::Function) {
      if (this->value.index() == 4) {
        return static_cast<T>(std::get<LuaCFunction>(this->value));
      } else {
        assert(this->value.index() == 9);
        return std::get<LuaFunction>(this->value).toCFunction();
      }
    } else {
      return defaultValue;
    }
  }

  template <typename T>
  typename std::enable_if<std::is_same<T, LuaTable>::value, T>::type
    LuaValue::get() const {
    if (this->type == LuaType::Table) {
      assert(this->value.index() == 5);
      return std::get<LuaTable>(this->value);
    } else {
      return LuaTable();
    }
  }

  template <typename T>
  typename std::enable_if<std::is_same<T, LuaUserData>::value, T>::type
    LuaValue::get() const {
    if (this->type == LuaType::UserData) {
      assert(this->value.index() == 7);
      return std::get<LuaUserData>(this->value);
    } else {
      return LuaUserData();
    }
  }

  template <typename T>
  typename std::enable_if<std::is_pointer<T>::value && std::is_class<typename std::remove_pointer<T>::type>::value, T>::type
    LuaValue::get(T defaultValue) const {
    using V = typename std::remove_pointer<T>::type;
    if (this->type == LuaType::UserData) {
      assert(this->value.index() == 7);
      const LuaUserData &data = std::get<LuaUserData>(this->value);
      V *custom_udata = data.getCustomData<V>();
      if (custom_udata) {
        return custom_udata;
      } else {
        LuaCppObjectWrapper<V> *ptr = data.toPointer<LuaCppObjectWrapper<V> *>();
        return ptr->get();
      }
    } else {
      return defaultValue;
    }
  }

  template <typename T>
  typename std::enable_if<std::is_reference<T>::value && std::is_class<typename std::decay<T>::type>::value && !Internal::LuaValueGetSpecialRef<T>::value, T>::type
    LuaValue::get() const {
    using V = typename std::decay<T>::type;
    if (this->type == LuaType::UserData) {
      assert(this->value.index() == 7);
      const LuaUserData &data = std::get<LuaUserData>(this->value);
      V *custom_udata = data.getCustomData<V>();
      if (custom_udata) {
        return *custom_udata;
      } else {
        LuaCppObjectWrapper<V> *ptr = std::get<LuaUserData>(this->value).toPointer<LuaCppObjectWrapper<V> *>();
        if (ptr != nullptr) {
          return *ptr->get();
        } else {
          throw LuaCppBError("Null pointer can't be dereferenced", LuaCppBErrorCode::NullPointerDereference);
        }
      }
    } else {
      throw LuaCppBError("Type casting failed", LuaCppBErrorCode::IncorrectTypeCast);
    }
  }

  template <typename T>
  typename std::enable_if<std::is_class<T>::value && !Internal::LuaValueGetSpecialCase<T>::value, T>::type
    LuaValue::get() const {
    using V = typename std::remove_reference<T>::type;
    if (this->type == LuaType::UserData) {
      assert(this->value.index() == 7);
      LuaCppObjectWrapper<V> *ptr = std::get<LuaUserData>(this->value).toPointer<LuaCppObjectWrapper<V> *>();
      if (ptr != nullptr) {
        return *ptr->get();
      } else {
        throw LuaCppBError("Null pointer can't be dereferenced", LuaCppBErrorCode::NullPointerDereference);
      }
    } else {
      throw LuaCppBError("Type casting failed", LuaCppBErrorCode::IncorrectTypeCast);
    }
  }

  template <typename T>
  typename std::enable_if<std::is_same<T, LuaThread>::value, T>::type
    LuaValue::get() const {
    if (this->type == LuaType::Thread) {
      assert(this->value.index() == 8);
      return std::get<LuaThread>(this->value);
    } else {
      return LuaThread();
    }
  }

  template <typename T>
  typename std::enable_if<std::is_same<T, LuaFunction>::value, T>::type
    LuaValue::get() const {
    if (this->type == LuaType::Function) {
      assert(this->value.index() == 9);
      return std::get<LuaFunction>(this->value);
    } else {
      return LuaFunction();
    }
  }

  template <typename T>
  typename std::enable_if<std::is_enum<T>::value, T>::type
    LuaValue::get() const {
    using EnumType = typename std::underlying_type<T>::type;
    return static_cast<T>(this->get<EnumType>());
  }

  template <typename T>
  typename std::enable_if<std::is_same<T, LuaEmpty>::value, T>::type
    LuaValue::get() const {
    return LuaEmpty {};
  }

  template <typename T, typename Type>
  LuaValue::operator T () {
    return this->get<Type>();
  }

  template <typename T, typename Type>
  LuaValue::operator T& () {
    return this->get<Type &>();
  }

  template <typename T>
  typename std::enable_if<std::is_integral<T>::value && !std::is_same<T, bool>::value, LuaValue>::type
    LuaValue::create(T value) noexcept {
    return LuaValue(LuaInteger(static_cast<lua_Integer>(value)));
  }

  template <typename T>
  typename std::enable_if<std::is_floating_point<T>::value, LuaValue>::type
    LuaValue::create(T value) noexcept {
    return LuaValue(LuaNumber(static_cast<lua_Number>(value)));
  }

  template <typename T>
  typename std::enable_if<std::is_same<T, bool>::value, LuaValue>::type
    LuaValue::create(T value) noexcept {
    return LuaValue(LuaBoolean(static_cast<bool>(value)));
  }

  template <typename T>
  typename std::enable_if<std::is_same<T, LuaCFunction>::value, LuaValue>::type
    LuaValue::create(T value) noexcept {
    return LuaValue(value);
  }

  template <typename T>
  typename std::enable_if<std::is_same<typename std::decay<T>::type, const char *>::value, LuaValue>::type
    LuaValue::create(T s) noexcept {
    return LuaValue(LuaString(s));
  }

  template <typename T>
  typename std::enable_if<std::is_same<typename std::decay<T>::type, std::string>::value, LuaValue>::type
    LuaValue::create(T s) noexcept {
    return LuaValue(LuaString(s));
  }
}

#endif
