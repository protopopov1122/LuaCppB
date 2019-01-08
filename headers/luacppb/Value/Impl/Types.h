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
        std::string className(lua_tostring(state, -1));
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