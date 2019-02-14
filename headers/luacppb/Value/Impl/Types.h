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

#ifndef LUACPPB_VALUE_IMPL_TYPES_H_
#define LUACPPB_VALUE_IMPL_TYPES_H_

#include "luacppb/Value/Types.h"
#include <sstream>

namespace LuaCppB {

  template <typename T>
  T LuaReferencedValue::toPointer() const {
    T res = nullptr;
    handle.get([&](lua_State *state) {
      const void *ptr = lua_topointer(state, -1);;
      res = reinterpret_cast<T>(const_cast<void *>(ptr));
    });
    return res;
  }

  template <typename T>
  std::string LuaUserData::getCustomName(uint64_t idx) {
    std::stringstream ss;
    ss << "$custom_udata_" << typeid(T).name() << '$' << idx;
    return ss.str();
  }

  template <typename T>
  T *LuaUserData::getCustomData() const {
    T *pointer = nullptr;
    this->handle.get([&](lua_State *state) {
      if (!lua_isuserdata(state, -1)) {
        return;
      }
      
      lua_getmetatable(state, -1);
      if (lua_istable(state, -1)) {
        lua_getfield(state, -1, "__name");
        const char *rawClassName = lua_tostring(state, -1);
        std::string className(rawClassName != nullptr ? rawClassName : "");
        lua_pop(state, 1);

        std::stringstream ss;
        ss << "$custom_udata_" << typeid(T).name();
        std::string userClassName(ss.str());
        if (className.compare(0, userClassName.size(), userClassName) == 0) {
          pointer = static_cast<T *>(lua_touserdata(state, -2));
        }
      }
      lua_pop(state, 1);
    });
    return pointer;
  }

  template <typename T>
  T LuaUserData::toPointer() const {
    return this->LuaReferencedValue::toPointer<T>();
  }
}

#endif
