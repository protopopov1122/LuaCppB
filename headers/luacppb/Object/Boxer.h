#ifndef LUACPPB_OBJECT_BOXER_H_
#define LUACPPB_OBJECT_BOXER_H_

#include "luacppb/Core/Error.h"
#include <typeindex>
#include <typeinfo>
#include <map>
#include <memory>

namespace LuaCppB {

  class LuaCppObjectBoxer {
   public:
    virtual ~LuaCppObjectBoxer() = default;
    virtual void wrap(lua_State *, void *) = 0;
    virtual void wrapUnique(lua_State *, void *) = 0;
  };

  class LuaCppObjectBoxerRegistry {
   public:
    LuaCppObjectBoxerRegistry(lua_State *state)
      : state(state) {}

    template <typename T>
    bool canWrap() {
      return this->wrappers.count(typeid(T)) != 0;
    }

    template <typename T>
    void wrap(lua_State *state, T *object) {
      if (state != this->state) {
        throw LuaCppBError("Lua state mismatch", LuaCppBErrorCode::StateMismatch);
      }
      if (this->canWrap<T>()) {
        this->wrappers[typeid(T)]->wrap(state, reinterpret_cast<void *>(object));
      }
    }

    template <typename T>
    void wrap(lua_State *state, std::unique_ptr<T> object) {
      if (state != this->state) {
        throw LuaCppBError("Lua state mismatch", LuaCppBErrorCode::StateMismatch);
      }
      if (this->canWrap<T>()) {
        T *pointer = object.release();
        this->wrappers[typeid(T)]->wrapUnique(state, reinterpret_cast<void *>(pointer));
      }
    }
   protected:
    template <typename T>
    void addBoxer(std::shared_ptr<LuaCppObjectBoxer> wrapper) {
      this->wrappers[typeid(T)] = wrapper;
    }
   protected:
    lua_State *state;
   private:
    std::map<std::type_index, std::shared_ptr<LuaCppObjectBoxer>> wrappers;
  };
}

#endif