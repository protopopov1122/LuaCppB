#ifndef LUACPPB_CONTAINER_CONTAINER_H_
#define LUACPPB_CONTAINER_CONTAINER_H_

#include "luacppb/Base.h"
#include "luacppb/Object/Wrapper.h"
#include "luacppb/Core/Stack.h"
#include <vector>

namespace LuaCppB {

  class LuaCppContainer {
   public:
    template <typename T, typename A>
    static void push(lua_State *state, LuaCppRuntime &runtime, std::vector<T, A> &vec, void (*pusher)(lua_State *state, LuaCppRuntime &, T &)) {
      using V = std::vector<T, A>;
      LuaCppObjectWrapper<V> *handle = reinterpret_cast<LuaCppObjectWrapper<V> *>(lua_newuserdata(state, sizeof(LuaCppObjectWrapper<V>)));
      new(handle) LuaCppObjectWrapper<V>(vec);
      lua_newtable(state);
      lua_pushlightuserdata(state, reinterpret_cast<void *>(&runtime));
      lua_pushlightuserdata(state, reinterpret_cast<void *>(pusher));
      lua_pushcclosure(state, &LuaCppContainer::vector_index<T, A>, 2);
      lua_setfield(state, -2, "__index");
      lua_pushcclosure(state, &LuaCppContainer::vector_length<T, A>, 0);
      lua_setfield(state, -2, "__len");
      lua_setmetatable(state, -2);
    }
   private:
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
  };
}

#endif