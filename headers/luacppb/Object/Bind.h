#ifndef LUACPPB_OBJECT_BIND_H_
#define LUACPPB_OBJECT_BIND_H_

#include "luacppb/Object/Class.h"

namespace LuaCppB {

  namespace Internal {
    template <typename C, typename ... T>
    struct ObjectBinder_Impl {};

    template <typename C>
    struct ObjectBinder_Impl<C> {
      static void bind(C &obj) {}
    };

    template <typename C, typename N, typename M, typename ... T>
    struct ObjectBinder_Impl<C, N, M, T...> {
      static void bind(C &obj, N &&name, M &&method, T &&... args) {
        obj.bind(name, method);
        ObjectBinder_Impl<C, T...>::bind(obj, std::forward<T>(args)...);
      }
    };

    template <typename C, typename ... T>
    struct ClassBinder_Impl {};

    template <typename C>
    struct ClassBinder_Impl<C> {
      static void bind(C &cl) {}
    };

    template <typename C, typename N, typename M, typename ... T>
    struct ClassBinder_Impl<C, N, M, T...> {
      static void bind(C &cl, N &&name, M &&method, T &&... args) {
        cl.bind(name, method);
        ClassBinder_Impl<C, T...>::bind(cl, std::forward<T>(args)...);
      }
    };
  }

  class ObjectBinder {
   public:
    template <typename C, typename ... T>
    static LuaCppObject<C> bind(C &obj, LuaCppRuntime &runtime, T &&... methods) {
      LuaCppObject<C> object(obj, runtime);
      Internal::ObjectBinder_Impl<LuaCppObject<C>, T...>::bind(object, std::forward<T>(methods)...);
      return object;
    }

    template <typename C, typename ... T>
    static LuaCppObject<C> bind(C *obj, LuaCppRuntime &runtime, T &&... methods) {
      LuaCppObject<C> object(obj, runtime);
      Internal::ObjectBinder_Impl<LuaCppObject<C>, T...>::bind(object, std::forward<T>(methods)...);
      return object;
    }
  };

  template <typename C, typename P = void>
  class ClassBinder {
   public:
    template <typename S, typename ... T>
    static LuaCppClass<C, P> bind(S &state, T &&... methods) {
      LuaCppClass<C, P> cl(state);
      Internal::ClassBinder_Impl<LuaCppClass<C, P>, T...>::bind(cl, std::forward<T>(methods)...);
      state.getClassRegistry().bind(cl);
      return cl;
    }
  };
}

#endif