#ifndef LUACPPB_OBJECT_REGISTRY_H_
#define LUACPPB_OBJECT_REGISTRY_H_

#include "luacppb/Object/Class.h"
#include "luacppb/Object/Boxer.h"
#include "luacppb/Core/Error.h"
#include "luacppb/Core/Stack.h"
#include <typeindex>
#include <typeinfo>

namespace LuaCppB::Internal {

  template <typename T, typename P>
  class LuaCppClassObjectBoxer : public LuaCppObjectBoxer {
   public:
    LuaCppClassObjectBoxer(const std::string &);

    void wrap(lua_State *, void *) override;
    void wrap(lua_State *, const void *) override;
    void wrapUnique(lua_State *, void *) override;
    void wrapShared(lua_State *, std::shared_ptr<void>) override;
    std::string getClassName() override;
   private:
    std::string className;
  };

  class LuaCppClassRegistry : public LuaCppObjectBoxerRegistry {
   public:
    using LuaCppObjectBoxerRegistry::LuaCppObjectBoxerRegistry;

    template <typename T, typename P>
    void bind(LuaCppClass<T, P> &);
  };
}

#include "luacppb/Object/Impl/Registry.h"

#endif
