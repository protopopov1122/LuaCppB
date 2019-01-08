#ifndef LUACPPB_OBJECT_BIND_H_
#define LUACPPB_OBJECT_BIND_H_

#include "luacppb/Object/Class.h"
#include "luacppb/Core/State.h"

namespace LuaCppB {

  namespace Internal {
    template <typename C, typename ... T>
    struct ObjectBinder_Impl {};

    template <typename C>
    struct ObjectBinder_Impl<C> {
      static void bind(C &obj);
    };

    template <typename C, typename N, typename M, typename ... T>
    struct ObjectBinder_Impl<C, N, M, T...> {
      static void bind(C &, N &&, M &&, T &&...);
    };

    template <typename C, typename ... T>
    struct ClassBinder_Impl {};

    template <typename C>
    struct ClassBinder_Impl<C> {
      static void bind(C &cl);
    };

    template <typename C, typename N, typename M, typename ... T>
    struct ClassBinder_Impl<C, N, M, T...> {
      static void bind(C &, N &&, M &&, T &&...);
    };
  }

  class ObjectBinder {
   public:
    template <typename C, typename ... T>
    static LuaCppObject<C> bind(C &, LuaCppRuntime &, T &&...);

    template <typename C, typename ... T>
    static LuaCppObject<C> bind(C *, LuaCppRuntime &, T &&...);
  };

  template <typename C, typename P = void>
  class ClassBinder {
   public:
    template <typename ... T>
    static LuaCppClass<C, P> bind(LuaState &, T &&...);

    template <typename ... T>
    static LuaCppClass<C, P> bind(const std::string &, LuaState &, T &&...);
  };
}

#include "luacppb/Object/Impl/Bind.h"

#endif