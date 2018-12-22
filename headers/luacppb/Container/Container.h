#ifndef LUACPPB_CONTAINER_CONTAINER_H_
#define LUACPPB_CONTAINER_CONTAINER_H_

#include "luacppb/Base.h"
#include "luacppb/Object/Wrapper.h"
#include "luacppb/Core/Stack.h"
#include <vector>
#include <typeinfo>
#include <type_traits>
#include <iostream>

namespace LuaCppB {

  template <typename T>
  struct is_container {
    static constexpr bool value = is_instantiation<std::vector, T>::value;
  };

  template <typename T, typename A, typename E = void>
  struct cpp_vector_newindex {
    static int newindex(lua_State *state) {
      return 0;
    }
  };

  template <typename T, typename A>
  struct cpp_vector_newindex<T, A, typename std::enable_if<std::is_same<decltype(std::declval<LuaValue>().get<T>()), T>::value>::type> {
    static int newindex(lua_State *state) {
      LuaStack stack(state);
      using V = std::vector<T, A>;
      using Handle = LuaCppObjectWrapper<V>;
      Handle *handle = stack.toPointer<Handle *>(1);
      std::size_t index = stack.toInteger(2) - 1;
      T value = stack.get(3).value_or(LuaValue()).get<T>();
      V &vec = *handle->get();
      vec[index] = value;
      return 0;
    }
  };

  class LuaCppContainer {
   public:
    template <typename T, typename A>
    static void push(lua_State *state, LuaCppRuntime &runtime, std::vector<T, A> &vec, void (*pusher)(lua_State *state, LuaCppRuntime &, T &)) {
      using V = std::vector<T, A>;
      LuaCppObjectWrapper<V> *handle = reinterpret_cast<LuaCppObjectWrapper<V> *>(lua_newuserdata(state, sizeof(LuaCppObjectWrapper<V>)));
      new(handle) LuaCppObjectWrapper<V>(vec);
      LuaCppContainer::set_vector_meta<T, A>(state, runtime, pusher);
    }

    template <typename T, typename A>
    static void push(lua_State *state, LuaCppRuntime &runtime, const std::vector<T, A> &vec, void (*pusher)(lua_State *state, LuaCppRuntime &, T &)) {
      using V = const std::vector<T, A>;
      LuaCppObjectWrapper<V> *handle = reinterpret_cast<LuaCppObjectWrapper<V> *>(lua_newuserdata(state, sizeof(LuaCppObjectWrapper<V>)));
      new(handle) LuaCppObjectWrapper<V>(vec);
      LuaCppContainer::set_vector_meta<T, A, const std::vector<T, A>>(state, runtime, pusher);
    }

    template <typename T, typename A>
    static void push(lua_State *state, LuaCppRuntime &runtime, std::unique_ptr<std::vector<T, A>> &vec, void (*pusher)(lua_State *state, LuaCppRuntime &, T &)) {
      using V = std::vector<T, A>;
      LuaCppObjectWrapper<V> *handle = reinterpret_cast<LuaCppObjectWrapper<V> *>(lua_newuserdata(state, sizeof(LuaCppObjectWrapper<V>)));
      new(handle) LuaCppObjectWrapper<V>(std::move(vec));
      LuaCppContainer::set_vector_meta<T, A>(state, runtime, pusher);
    }

    template <typename T, typename A>
    static void push(lua_State *state, LuaCppRuntime &runtime, std::shared_ptr<std::vector<T, A>> &vec, void (*pusher)(lua_State *state, LuaCppRuntime &, T &)) {
      using V = std::vector<T, A>;
      LuaCppObjectWrapper<V> *handle = reinterpret_cast<LuaCppObjectWrapper<V> *>(lua_newuserdata(state, sizeof(LuaCppObjectWrapper<V>)));
      new(handle) LuaCppObjectWrapper<V>(vec);
      LuaCppContainer::set_vector_meta<T, A>(state, runtime, pusher);
    }
   private:
    template <typename T, typename A, typename V = std::vector<T, A>>
    static void set_vector_meta(lua_State *state, LuaCppRuntime &runtime, void (*pusher)(lua_State *state, LuaCppRuntime &, T &)) {
      std::string typeName = typeid(V).name();
      if constexpr (std::is_const<V>::value) {
        typeName = typeName + "__const";
      }
      if (luaL_newmetatable(state, typeName.c_str())) {
        lua_pushlightuserdata(state, reinterpret_cast<void *>(&runtime));
        lua_pushlightuserdata(state, reinterpret_cast<void *>(pusher));
        lua_pushcclosure(state, &LuaCppContainer::vector_index<T, A>, 2);
        lua_setfield(state, -2, "__index");
        if constexpr (!std::is_const<V>::value) {
          lua_pushcclosure(state, &cpp_vector_newindex<T, A>::newindex, 0);
          lua_setfield(state, -2, "__newindex");
        }
        lua_pushcclosure(state, &LuaCppContainer::vector_length<T, A>, 0);
        lua_setfield(state, -2, "__len");
        lua_pushcclosure(state, &LuaCppContainer::vector_gc<T, A>, 0);
        lua_setfield(state, -2, "__gc");
      }
      lua_setmetatable(state, -2);
    }

    template <typename T, typename A>
    static int vector_index(lua_State *state) {
      LuaStack stack(state);
      using V = std::vector<T, A>;
      using Pusher = void (*)(lua_State *state, LuaCppRuntime &, T &);
      using Handle = LuaCppObjectWrapper<V>;
      Handle *handle = stack.toPointer<Handle *>(1);
      LuaCppRuntime &runtime = *stack.toPointer<LuaCppRuntime *>(lua_upvalueindex(1));
      Pusher pusher = stack.toPointer<Pusher>(lua_upvalueindex(2));
      std::size_t index = stack.toInteger(2) - 1;
      V &vec = *handle->get();
      if (index >= vec.size()) {
        stack.push();
      } else {
        pusher(state, runtime, vec.at(index));
      }
      return 1;
    }

    template <typename T, typename A>
    static int vector_length(lua_State *state) {
      LuaStack stack(state);
      using V = std::vector<T, A>;
      using Handle = LuaCppObjectWrapper<V>;
      Handle *handle = stack.toPointer<Handle *>(1);
      V &vec = *handle->get();
      stack.push(vec.size());
      return 1;
    }

    template <typename T, typename A>
    static int vector_gc(lua_State *state) {
      LuaStack stack(state);
      using V = std::vector<T, A>;
      using Handle = LuaCppObjectWrapper<V>;
      Handle *handle = stack.toPointer<Handle *>(1);
      if (handle) {
        handle->~LuaCppObjectWrapper();
        ::operator delete(handle, handle);
      }
      return 0;
    }
  };
}

#endif