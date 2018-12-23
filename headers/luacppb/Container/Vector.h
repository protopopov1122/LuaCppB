#ifndef LUACPPB_CONTAINER_VECTOR_H_
#define LUACPPB_CONTAINER_VECTOR_H_

#include "luacppb/Base.h"
#include "luacppb/Object/Wrapper.h"
#include "luacppb/Core/Stack.h"
#include <vector>
#include <typeinfo>
#include <type_traits>
#include <iostream>

namespace LuaCppB {

  template <typename T, typename A, typename E = void>
  struct cpp_vector_newindex {
    static int newindex(lua_State *state) {
      return 0;
    }
  };

  template <typename T, typename A>
  struct cpp_vector_newindex<T, A, typename std::enable_if<std::is_same<decltype(std::declval<LuaValue>().get<T>()), T>::value>::type> {
    static int newindex(lua_State *state) {
      using V = std::vector<T, A>;
      LuaStack stack(state);
      using Handle = LuaCppObjectWrapper<V>;
      Handle *handle = stack.toPointer<Handle *>(1);
      std::size_t index = stack.toInteger(2) - 1;
      T value = stack.get(3).value_or(LuaValue()).get<T>();
      V &vec = *handle->get();
      vec[index] = value;
      return 0;
    }
  };

  template <class P>
  class LuaCppVector {
   public:
    template <typename V>
    static typename std::enable_if<is_instantiation<std::vector, V>::value>::type
      push(lua_State *state, LuaCppRuntime &runtime, V &vec) {
      LuaCppObjectWrapper<V> *handle = reinterpret_cast<LuaCppObjectWrapper<V> *>(lua_newuserdata(state, sizeof(LuaCppObjectWrapper<V>)));
      new(handle) LuaCppObjectWrapper<V>(vec);
      LuaCppVector<P>::set_vector_meta<V>(state, runtime);
    }

    template <typename V>
    static typename std::enable_if<is_instantiation<std::vector, V>::value>::type
      push(lua_State *state, LuaCppRuntime &runtime, const V &vec) {
      LuaCppObjectWrapper<const V> *handle = reinterpret_cast<LuaCppObjectWrapper<const V> *>(lua_newuserdata(state, sizeof(LuaCppObjectWrapper<V>)));
      new(handle) LuaCppObjectWrapper<const V>(vec);
      LuaCppVector<P>::set_vector_meta<const V>(state, runtime);
    }

    template <typename V>
    static typename std::enable_if<is_instantiation<std::vector, V>::value>::type
      push(lua_State *state, LuaCppRuntime &runtime, std::unique_ptr<V> &vec) {
      LuaCppObjectWrapper<V> *handle = reinterpret_cast<LuaCppObjectWrapper<V> *>(lua_newuserdata(state, sizeof(LuaCppObjectWrapper<V>)));
      new(handle) LuaCppObjectWrapper<V>(std::move(vec));
      LuaCppVector<P>::set_vector_meta<V>(state, runtime);
    }

    template <typename V>
    static typename std::enable_if<is_instantiation<std::vector, V>::value>::type
      push(lua_State *state, LuaCppRuntime &runtime, std::shared_ptr<V> &vec) {
      LuaCppObjectWrapper<V> *handle = reinterpret_cast<LuaCppObjectWrapper<V> *>(lua_newuserdata(state, sizeof(LuaCppObjectWrapper<V>)));
      new(handle) LuaCppObjectWrapper<V>(vec);
      LuaCppVector<P>::set_vector_meta<V>(state, runtime);
    }
   private:
    template <typename V>
    static void set_vector_meta(lua_State *state, LuaCppRuntime &runtime) {
      using T = typename V::value_type;
      using A = typename V::allocator_type;
      std::string typeName = typeid(V).name();
      if constexpr (std::is_const<V>::value) {
        typeName = typeName + "__const";
      }
      if (luaL_newmetatable(state, typeName.c_str())) {
        lua_pushlightuserdata(state, reinterpret_cast<void *>(&runtime));
        lua_pushcclosure(state, &LuaCppVector<P>::vector_index<V>, 1);
        lua_setfield(state, -2, "__index");
        if constexpr (!std::is_const<V>::value) {
          lua_pushcclosure(state, &cpp_vector_newindex<T, A>::newindex, 0);
          lua_setfield(state, -2, "__newindex");
        }
        lua_pushlightuserdata(state, reinterpret_cast<void *>(&runtime));
        lua_pushcclosure(state, &LuaCppVector<P>::vector_pairs<V>, 1);
        lua_setfield(state, -2, "__pairs");
        lua_pushcclosure(state, &LuaCppVector<P>::vector_length<V>, 0);
        lua_setfield(state, -2, "__len");
        lua_pushcclosure(state, &LuaCppVector<P>::vector_gc<V>, 0);
        lua_setfield(state, -2, "__gc");
      }
      lua_setmetatable(state, -2);
    }

    template <typename V>
    static int vector_index(lua_State *state) {
      LuaStack stack(state);
      using Handle = LuaCppObjectWrapper<V>;
      Handle *handle = stack.toPointer<Handle *>(1);
      LuaCppRuntime &runtime = *stack.toPointer<LuaCppRuntime *>(lua_upvalueindex(1));
      std::size_t index = stack.toInteger(2) - 1;
      V &vec = *handle->get();
      if (index >= vec.size()) {
        return 0;
      } else {
        P::push(state, runtime, vec.at(index));
        return 1;
      }
    }

    template <typename V>
    static int vector_length(lua_State *state) {
      LuaStack stack(state);
      using Handle = LuaCppObjectWrapper<V>;
      Handle *handle = stack.toPointer<Handle *>(1);
      V &vec = *handle->get();
      stack.push(vec.size());
      return 1;
    }

    template <typename V>
    static int vector_gc(lua_State *state) {
      LuaStack stack(state);
      using Handle = LuaCppObjectWrapper<V>;
      Handle *handle = stack.toPointer<Handle *>(1);
      if (handle) {
        handle->~LuaCppObjectWrapper();
        ::operator delete(handle, handle);
      }
      return 0;
    }

    template <typename V>
    static int vector_pairs(lua_State *state) {
      lua_pushvalue(state, lua_upvalueindex(1));
      lua_pushcclosure(state, &LuaCppVector<P>::vector_iter<V>, 1);
      lua_pushvalue(state, 1);
      lua_pushnil(state);
      return 3;
    }

    template <typename V>
    static int vector_iter(lua_State *state) {
      using Handle = LuaCppObjectWrapper<V>;
      LuaStack stack(state);
      LuaCppRuntime &runtime = *stack.toPointer<LuaCppRuntime *>(lua_upvalueindex(1));
      Handle *handle = stack.toPointer<Handle *>(1);
      V *vec = handle->get();
      if (vec == nullptr) {
        return 0;
      }
      std::size_t index = 0;
      typename V::const_iterator iter = vec->begin();
      if (!lua_isnil(state, 2)) {
        index = stack.toInteger(2);
        std::size_t key = index;
        while (key--) {
          ++iter;
        }
      }
      if (iter != vec->end()) {
        P::push(state, runtime, index + 1);
        P::push(state, runtime, *iter);
        return 2;
      } else {
        return 0;
      }
    }
  };
}

#endif