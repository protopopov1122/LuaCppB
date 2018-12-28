#ifndef LUACPPB_OBJECT_BOXER_H_
#define LUACPPB_OBJECT_BOXER_H_

#include "luacppb/Core/Error.h"
#include <typeindex>
#include <typeinfo>
#include <map>
#include <memory>

namespace LuaCppB::Internal {

  class LuaCppObjectBoxer {
  public:
    virtual ~LuaCppObjectBoxer() = default;
    virtual void wrap(lua_State *, void *) = 0;
    virtual void wrap(lua_State *, const void *) = 0;
    virtual void wrapUnique(lua_State *, void *) = 0;
    virtual void wrapShared(lua_State *, std::shared_ptr<void>) = 0;
    virtual std::string getClassName() = 0;
  };

  class LuaCppObjectBoxerRegistry {
   public:
    LuaCppObjectBoxerRegistry(lua_State *state)
      : state(state) {}
    
    template <typename T>
    std::string getClassName() {
      std::type_index idx = std::type_index(typeid(T));
      if (this->wrappers.count(idx)) {
        return this->wrappers[idx]->getClassName();
      } else {
        return "";
      }
    }

    template <typename T>
    bool canWrap() {
      return this->wrappers.count(typeid(T)) != 0;
    }

    template <typename T>
    void wrap(lua_State *state, T *object) {
      if (this->canWrap<T>()) {
        if constexpr (std::is_const<T>::value) {
          this->wrappers[typeid(T)]->wrap(state, reinterpret_cast<const void *>(object));
        } else {
          this->wrappers[typeid(T)]->wrap(state, reinterpret_cast<void *>(object));
        }
      }
    }

    template <typename T>
    void wrap(lua_State *state, std::unique_ptr<T> object) {
      if (this->canWrap<T>()) {
        T *pointer = object.release();
        this->wrappers[typeid(T)]->wrapUnique(state, reinterpret_cast<void *>(pointer));
      }
    }

    template <typename T>
    void wrap(lua_State *state, std::shared_ptr<T> object) {
      if (this->canWrap<T>()) {
        this->wrappers[typeid(T)]->wrapShared(state, std::reinterpret_pointer_cast<void >(object));
      }
    }
   protected:
    template <typename T>
    void addBoxer(std::shared_ptr<Internal::LuaCppObjectBoxer> wrapper) {
      this->wrappers[typeid(T)] = wrapper;
    }
   protected:
    lua_State *state;
   private:
    std::map<std::type_index, std::shared_ptr<Internal::LuaCppObjectBoxer>> wrappers;
  };
}

#endif