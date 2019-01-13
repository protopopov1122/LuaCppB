#ifndef LUACPPB_VALUE_FACTORY_H_
#define LUACPPB_VALUE_FACTORY_H_

#include "luacppb/Base.h"
#include "luacppb/Value/Native.h"
#include "luacppb/Reference/Handle.h"

namespace LuaCppB {


  class LuaValueFactory {
   public:
    template <typename T>
    static LuaReferenceHandle newTable(T &);

    template <typename T, typename F>
    static LuaReferenceHandle newFunction(T &, F &&);

#ifdef LUACPPB_COROUTINE_SUPPORT
    template <typename T>
    static LuaReferenceHandle newThread(T &, LuaReferenceHandle);
#endif

    template <typename T, typename V>
    static LuaValue wrap(T &, V &);

    template <typename T, typename V>
    static LuaValue wrap(T &, V &&);
  };
}

#include "luacppb/Value/Impl/Factory.h"

#endif