#ifndef LUACPPB_CORE_IMPL_DEBUG_H_
#define LUACPPB_CORE_IMPL_DEBUG_H_

#include "luacppb/Core/Debug.h"

namespace LuaCppB {

  namespace Internal {
    template <typename F>
    void LuaDebugFunction_Impl<F>::getWhat(std::string &line) {}

    template <typename F, typename A, typename ... B>
    void LuaDebugFunction_Impl<F, A, B...>::getWhat(std::string &line, A a, B... b) {
      switch (a) {
        case F::Name:
          line += 'n';
          break;
        case F::Source:
          line += 'S';
          break;
        case F::Line:
          line += 'l';
          break;
        case F::Params:
          line += 'u';
          break;
      }
      LuaDebugFunction_Impl<F, B...>::getWhat(line, b...);
    }
  }

  template <typename Reference, typename ReferenceInternal>
  template <typename ... T>
  void LuaAbstractDebugFrame<Reference, ReferenceInternal>::getFunctionInfo(Reference ref, T ... args) {
    std::string what = ">";
    Internal::LuaDebugFunction_Impl<Function, T...>::getWhat(what, args...);
    ref.getReference().putOnTop([&](lua_State *state) {
      Internal::LuaStack stack(state);
      stack.copy(-1);
      if (state == this->state) {
        lua_getinfo(state, what.c_str(), &this->debug);
      } else {
        stack.move(this->state, 1);
        lua_getinfo(this->state, what.c_str(), &this->debug);
      }
    });
  }

  template <typename Reference, typename ReferenceInternal>
  template <typename ... T>
  void LuaAbstractDebugFrame<Reference, ReferenceInternal>::getCurrentFunctionInfo(T ... args) {
    std::string what;
    Internal::LuaDebugFunction_Impl<Function, T...>::getWhat(what, args...);
    lua_getinfo(this->state, what.c_str(), &this->debug);
  }

  template <typename Reference, typename ReferenceInternal>
  std::optional<typename LuaAbstractDebugFrame<Reference, ReferenceInternal>::Variable> LuaAbstractDebugFrame<Reference, ReferenceInternal>::getLocal(int index) {
    const char *key = lua_getlocal(this->state, &this->debug, index);
    if (key == nullptr) {
      return std::optional<Variable>();
    } else {
      Reference value(this->state, std::make_unique<ReferenceInternal>(this->state, this->runtime, -1));
      lua_pop(this->state, 1);
      return Variable { std::string(key), value };
    }
  }

  template <typename Reference, typename ReferenceInternal>
  std::optional<std::string> LuaAbstractDebugFrame<Reference, ReferenceInternal>::getLocal(Reference ref, int index) {
    const char *key = nullptr;
    ref.getReference().putOnTop([&](lua_State *state) {
      key = lua_getlocal(state, nullptr, index);
    });
    if (key) {
      return std::string(key);
    } else {
      return std::optional<std::string>();
    }
  }

  template <typename Reference, typename ReferenceInternal>
  std::optional<typename LuaAbstractDebugFrame<Reference, ReferenceInternal>::Variable> LuaAbstractDebugFrame<Reference, ReferenceInternal>::getUpvalue(Reference ref, int index) {
    const char *key = nullptr;
    Reference result;
    ref.getReference().putOnTop([&](lua_State *state) {
      key = lua_getupvalue(state, -1, index);
      if (key) {
        result = Reference(this->state, std::make_unique<ReferenceInternal>(state, this->runtime, -1));
        lua_pop(state, 1);
      }
    });
    if (key) {
      return Variable { std::string(key), result };
    } else {
      return std::optional<Variable>();
    }
  }

  template <typename Reference, typename ReferenceInternal>
  typename LuaAbstractDebugFrame<Reference, ReferenceInternal>::UpvalueId LuaAbstractDebugFrame<Reference, ReferenceInternal>::getUpvalueId(Reference ref, int index) {
    UpvalueId id = nullptr;
    ref.getReference().putOnTop([&](lua_State *state) {
      id = lua_upvalueid(state, -1, index);
    });
    return id;
  }
}

#endif