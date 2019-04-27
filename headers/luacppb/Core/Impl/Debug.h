/*
  SPDX short identifier: MIT

  Copyright 2018-2019 Jevgēnijs Protopopovs

  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"),
  to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
  and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
  IN THE SOFTWARE.
*/

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

#ifdef LUACPPB_DEBUG_EXTRAS_SUPPORT
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
  typename LuaAbstractDebugFrame<Reference, ReferenceInternal>::UpvalueId LuaAbstractDebugFrame<Reference, ReferenceInternal>::getUpvalueId(Reference ref, int index) {
    UpvalueId id = nullptr;
    ref.getReference().putOnTop([&](lua_State *state) {
      id = lua_upvalueid(state, -1, index);
    });
    return id;
  }

  template <typename Reference, typename ReferenceInternal>
  void LuaAbstractDebugFrame<Reference, ReferenceInternal>::joinUpvalues(Reference fn1, int idx1, Reference fn2, int idx2) {
    Internal::LuaStackGuard guard(this->state);
    auto canary = guard.canary();
    fn1.getReference().putOnTop([&](lua_State *state1) {
      if (state1 != this->state) {
        Internal::LuaStack fstack(state1);
        fstack.copy(-1);
        fstack.move(this->state, 1);
      }
      fn2.getReference().putOnTop([&](lua_State *state2) {
        if (state2 != this->state) {
          Internal::LuaStack fstack(state1);
          fstack.copy(-1);
          fstack.move(this->state, 1);
        }
        lua_upvaluejoin(this->state, -2, idx1, -1, idx2);
        if (state2 != this->state) {
          lua_pop(this->state, 1);
        }
      });
      if (state1 != this->state) {
        lua_pop(this->state, 1);
      }
    });
    canary.assume();
  }
#endif

  template <typename Reference, typename ReferenceInternal>
  bool LuaAbstractDebugFrame<Reference, ReferenceInternal>::setLocal(int index, Reference value) {
    bool result = false;
    value.getReference().putOnTop([&](lua_State *state) {
      Internal::LuaStack stack(state);
      int top = stack.getTop();
      stack.copy(-1);
      if (state == this->state) {
        result = lua_setlocal(state, &this->debug, index) != nullptr;
        if (!result && stack.getTop() > top) {
          stack.pop(stack.getTop() - top);
        }
      } else {
        Internal::LuaStack stateStack(this->state);
        top = stateStack.getTop();
        stack.move(this->state, 1);
        result = lua_setlocal(this->state, &this->debug, index) != nullptr;
        if (!result && stateStack.getTop() > top) {
          lua_pop(this->state, stateStack.getTop() - top);
        }
      }
    });
    return result;
  }

  template <typename Reference, typename ReferenceInternal>
  bool LuaAbstractDebugFrame<Reference, ReferenceInternal>::setUpvalue(Reference func, int index, Reference value) {
    bool result = false;
    Internal::LuaStackGuard guard(this->state);
    auto canary = guard.canary();
    func.getReference().putOnTop([&](lua_State *fstate) {
      if (fstate != this->state) {
        Internal::LuaStack fstack(fstate);
        fstack.copy(-1);
        fstack.move(this->state, 1);
      }
      value.getReference().putOnTop([&](lua_State *vstate) {
        Internal::LuaStack vstack(vstate);
        vstack.copy(-1);
        if (vstate == this->state) {
          result = lua_setupvalue(this->state, -3, index) != nullptr;
          if (!result) {
            lua_pop(this->state, 1);
          }
        } else {
          vstack.move(this->state, 1);
          result = lua_setupvalue(this->state, -2, index) != nullptr;
          if (!result) {
            lua_pop(this->state, 1);
          }
        }
      });
      if (fstate != this->state) {
        lua_pop(this->state, 1);
      }
    });
    canary.assume();
    return result;
  }

  template <typename Reference, typename ReferenceInternal>
  Reference LuaAbstractDebugFrame<Reference, ReferenceInternal>::getCurrentFunction() {
    int top = lua_gettop(this->state);
    lua_getinfo(this->state, "f", &this->debug);
    int total = lua_gettop(this->state) - top;
    if (total > 0) {
      Reference res(this->state, std::make_unique<ReferenceInternal>(this->state, this->runtime, -1));
      lua_pop(this->state, total);
      return res;
    } else {
      return Reference();
    }
  }

  template <typename Reference, typename ReferenceInternal>
  Reference LuaAbstractDebugFrame<Reference, ReferenceInternal>::getLines(Reference fn) {
    Reference res;
    fn.getReference().putOnTop([&](lua_State *state) {
      Internal::LuaStack stack(state);
      int top = stack.getTop();
      stack.copy(-1);
      lua_getinfo(state, ">L", &this->debug);
      int total = stack.getTop() - top;
      if (total > 0) {
        res = Reference(state, std::make_unique<ReferenceInternal>(state, this->runtime, -1));
        stack.pop(total);
      }
    });
    return res;
  }

  template <typename Reference, typename ReferenceInternal>
  Reference LuaAbstractDebugFrame<Reference, ReferenceInternal>::getLines() {
    int top = lua_gettop(this->state);
    lua_getinfo(this->state, "L", &this->debug);
    int total = lua_gettop(this->state) - top;
    if (total > 0) {
      Reference res(this->state, std::make_unique<ReferenceInternal>(this->state, this->runtime, -1));
      lua_pop(this->state, total);
      return res;
    } else {
      return Reference();
    }
  }

  template <typename Debug>
  LuaDebugAbstractHooks<Debug>::LuaDebugAbstractHooks(lua_State *state, LuaCppRuntime &runtime)
    : state(state), runtime(runtime) {
    Internal::LuaDebugHookDispatcher::getGlobal().attach(this->state);
  }

  template <typename Debug>
  LuaDebugAbstractHooks<Debug>::LuaDebugAbstractHooks(LuaDebugAbstractHooks &&hooks)
    : state(hooks.state) {
    hooks.state = nullptr;
  }

  template <typename Debug>
  LuaDebugAbstractHooks<Debug>::~LuaDebugAbstractHooks() {
    if (this->state != nullptr) {
      Internal::LuaDebugHookDispatcher::getGlobal().detach(this->state);
    }
  }

  template <typename Debug>
  LuaDebugAbstractHooks<Debug> &LuaDebugAbstractHooks<Debug>::operator=(LuaDebugAbstractHooks &&hooks) {
    this->state = hooks.state;
    hooks.state = nullptr;
    return *this;
  }
  
  template <typename Debug>
  typename LuaDebugAbstractHooks<Debug>::Detach LuaDebugAbstractHooks<Debug>::onCall(Hook hook) {
    if (this->state) {
      return Internal::LuaDebugHookDispatcher::getGlobal().attachCall(this->state, [this, hook](lua_State *state, lua_Debug *debug) {
        Debug dbg(state, this->runtime, debug);
        hook(dbg);
      });
    } else {
      return Detach();
    }
  }
  
  template <typename Debug>
  typename LuaDebugAbstractHooks<Debug>::Detach LuaDebugAbstractHooks<Debug>::onReturn(Hook hook) {
    if (this->state) {
      return Internal::LuaDebugHookDispatcher::getGlobal().attachReturn(this->state, [this, hook](lua_State *state, lua_Debug *debug) {
        Debug dbg(state, this->runtime, debug);
        hook(dbg);
      });
    } else {
      return Detach();
    }
  }
  
  template <typename Debug>
  typename LuaDebugAbstractHooks<Debug>::Detach LuaDebugAbstractHooks<Debug>::onLine(Hook hook) {
    if (this->state) {
      return Internal::LuaDebugHookDispatcher::getGlobal().attachLine(this->state, [this, hook](lua_State *state, lua_Debug *debug) {
        Debug dbg(state, this->runtime, debug);
        hook(dbg);
      });
    } else {
      return Detach();
    }
  }
  
  template <typename Debug>
  typename LuaDebugAbstractHooks<Debug>::Detach LuaDebugAbstractHooks<Debug>::onCount(Hook hook, unsigned int counter) {
    if (this->state) {
      return Internal::LuaDebugHookDispatcher::getGlobal().attachCount(this->state, [this, hook](lua_State *state, lua_Debug *debug) {
        Debug dbg(state, this->runtime, debug);
        hook(dbg);
      }, counter);
    } else {
      return Detach();
    }
  }
}

#endif