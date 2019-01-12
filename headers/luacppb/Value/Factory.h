#ifndef LUACPPB_VALUE_FACTORY_H_
#define LUACPPB_VALUE_FACTORY_H_

#include "luacppb/Base.h"
#include "luacppb/Value/Value.h"
#include "luacppb/Reference/Handle.h"

namespace LuaCppB {


  class LuaValueFactory {
   public:
    template <typename T>
    static LuaReferenceHandle newTable(T &);

    template <typename T, typename F>
    static LuaReferenceHandle newFunction(T &, F &&);

    template <typename T>
    static LuaReferenceHandle newThread(T &env, LuaReferenceHandle);
  };
}

#include "luacppb/Value/Impl/Factory.h"

#endif