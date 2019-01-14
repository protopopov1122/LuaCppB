#ifndef LUACPPB_OBJECT_BOXER_H_
#define LUACPPB_OBJECT_BOXER_H_

#include "luacppb/Core/Error.h"
#include <typeindex>
#include <typeinfo>
#include <map>
#include <memory>

namespace LuaCppB::Internal {

  class LuaCppObjectFieldPusher;

  class LuaCppObjectBoxer {
  public:
    virtual ~LuaCppObjectBoxer() = default;
    virtual void wrap(lua_State *, void *) = 0;
    virtual void wrap(lua_State *, const void *) = 0;
    virtual void wrapUnique(lua_State *, void *) = 0;
    virtual void wrapShared(lua_State *, std::shared_ptr<void>) = 0;
    virtual std::string getClassName() = 0;
    virtual void copyFields(std::map<std::string, std::shared_ptr<LuaCppObjectFieldPusher>> &) = 0;
  };

  class LuaCppObjectBoxerRegistry {
   public:
    LuaCppObjectBoxerRegistry(lua_State *state)
      : state(state) {}
    
    template <typename T>
    std::string getClassName();

    template <typename T>
    void copyFields(std::map<std::string, std::shared_ptr<LuaCppObjectFieldPusher>> &);

    template <typename T>
    bool canWrap();

    template <typename T>
    void wrap(lua_State *, T *);

    template <typename T>
    void wrap(lua_State *, std::unique_ptr<T>);

    template <typename T>
    void wrap(lua_State *, std::shared_ptr<T>);
   protected:
    template <typename T>
    void addBoxer(std::shared_ptr<Internal::LuaCppObjectBoxer>);

    lua_State *state;
   private:
    std::map<std::type_index, std::shared_ptr<Internal::LuaCppObjectBoxer>> wrappers;
  };
}

#include "luacppb/Object/Impl/Boxer.h"

#endif
