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

#include "luacppb/Core/Debug.h"

#ifdef LUACPPB_DEBUG_SUPPORT

namespace LuaCppB {

  LuaDebugBaseFrame::LuaDebugBaseFrame(lua_State *state, LuaCppRuntime &runtime, lua_Debug *debug)
    : state(state), runtime(runtime), debug(*debug) {}

  LuaDebugBaseFrame::LuaDebugBaseFrame(lua_State *state, LuaCppRuntime &runtime, int level)
    : state(state), runtime(runtime) {
    lua_getstack(state, level, &this->debug);
  }

  int LuaDebugBaseFrame::getEvent() const {
    return this->debug.event;
  }

  std::string LuaDebugBaseFrame::getName() const {
    return std::string(this->debug.name != nullptr ? this->debug.name : "");
  }

  std::string LuaDebugBaseFrame::getNameWhat() const {
    return std::string(this->debug.namewhat);
  }

  std::string LuaDebugBaseFrame::getWhat() const {
    return std::string(this->debug.what);
  }

  std::string LuaDebugBaseFrame::getSource() const {
    return std::string(this->debug.source);
  }

  std::string LuaDebugBaseFrame::getShortSource() const {
    return std::string(this->debug.short_src);
  }

  int LuaDebugBaseFrame::getCurrentLine() const {
    return this->debug.currentline;
  }

  int LuaDebugBaseFrame::getLineDefined() const {
    return this->debug.linedefined;
  }

  int LuaDebugBaseFrame::getLastLineDefined() const {
    return this->debug.lastlinedefined;
  }

  unsigned char LuaDebugBaseFrame::getUpvalues() const {
    return this->debug.nups;
  }

#ifdef LUACPPB_DEBUG_EXTRAS_SUPPORT

  unsigned char LuaDebugBaseFrame::getParameters() const {
    return this->debug.nparams;
  }

  bool LuaDebugBaseFrame::isVarArg() const {
    return static_cast<bool>(this->debug.isvararg);
  }

  bool LuaDebugBaseFrame::isTailCall() const {
    return static_cast<bool>(this->debug.istailcall);
  }

#endif

  namespace Internal {

    LuaDebugHookDispatcher LuaDebugHookDispatcher::singleton;

    LuaDebugHookDispatcher::DetachHook::DetachHook()
      : dispatcher(LuaDebugHookDispatcher::getGlobal()), type(Type::Call) {}

    LuaDebugHookDispatcher::DetachHook::DetachHook(LuaDebugHookDispatcher &dispatcher, Type type, std::size_t id)
      : dispatcher(dispatcher), type(type), id(id) {}
    
    LuaDebugHookDispatcher::DetachHook::DetachHook(DetachHook &&hook)
      : dispatcher(hook.dispatcher), type(hook.type), id(hook.id) {
      hook.id.reset();
    }

    LuaDebugHookDispatcher::DetachHook &LuaDebugHookDispatcher::DetachHook::operator=(DetachHook &&hook) {
      this->dispatcher = hook.dispatcher;
      this->type = hook.type;
      this->id = hook.id;
      hook.id.reset();
      return *this;
    }

    bool LuaDebugHookDispatcher::DetachHook::attached() const {
      return this->id.has_value();
    }
    
    void LuaDebugHookDispatcher::DetachHook::operator()() {
      if (this->id.has_value()) {
        lua_Hook hookFn;
        std::map<lua_State *, std::set<std::size_t>> *map;
        switch (this->type) {
          case Type::Call:
            hookFn = &LuaDebugHookDispatcher::callHook;
            map = &this->dispatcher.get().callHooks;
            break;
          case Type::Count:
            hookFn = &LuaDebugHookDispatcher::countHook;
            map = &this->dispatcher.get().countHooks;
            this->dispatcher.get().counter.erase(this->id.value());
            break;
          case Type::Line:
            hookFn = &LuaDebugHookDispatcher::lineHook;
            map = &this->dispatcher.get().lineHooks;
            break;
          case Type::Return:
            hookFn = &LuaDebugHookDispatcher::returnHook;
            map = &this->dispatcher.get().returnHooks;
            break;
        }
        auto hook = this->dispatcher.get().hooks[this->id.value()];
        this->dispatcher.get().hooks.erase(this->id.value());
        (*map)[hook.first].erase(this->id.value());
        this->dispatcher.get().unbindState(*map, hook.first, hookFn);
        this->id.reset();
      }
    }

    LuaDebugHookDispatcher::LuaDebugHookDispatcher()
      : nextId(0) {}

    LuaDebugHookDispatcher::DetachHook LuaDebugHookDispatcher::attachCall(lua_State *state, Hook hook) {
      std::size_t id = this->nextId++;
      this->hooks[id] = std::make_pair(state, hook);
      this->bindState(this->callHooks, state, &LuaDebugHookDispatcher::callHook, LUA_MASKCALL);
      this->callHooks[state].emplace(id);
      return DetachHook(*this, DetachHook::Type::Call, id);
    }

    LuaDebugHookDispatcher::DetachHook LuaDebugHookDispatcher::attachReturn(lua_State *state, Hook hook) {
      std::size_t id = this->nextId++;
      this->hooks[id] = std::make_pair(state, hook);
      this->bindState(this->returnHooks, state, &LuaDebugHookDispatcher::returnHook, LUA_MASKRET);
      this->returnHooks[state].emplace(id);
      return DetachHook(*this, DetachHook::Type::Return, id);
    }

    LuaDebugHookDispatcher::DetachHook LuaDebugHookDispatcher::attachLine(lua_State *state, Hook hook) {
      std::size_t id = this->nextId++;
      this->hooks[id] = std::make_pair(state, hook);
      this->bindState(this->lineHooks, state, &LuaDebugHookDispatcher::lineHook, LUA_MASKLINE);
      this->lineHooks[state].emplace(id);
      return DetachHook(*this, DetachHook::Type::Line, id);
    }

    LuaDebugHookDispatcher::DetachHook LuaDebugHookDispatcher::attachCount(lua_State *state, Hook hook, unsigned int counter) {
      std::size_t id = this->nextId++;
      this->hooks[id] = std::make_pair(state, hook);
      this->bindState(this->countHooks, state, &LuaDebugHookDispatcher::countHook, LUA_MASKCOUNT);
      this->countHooks[state].emplace(id);
      this->counter[id] = std::make_pair(1, counter);
      return DetachHook(*this, DetachHook::Type::Count, id);
    }

    void LuaDebugHookDispatcher::attach(lua_State *state) {
      if (this->attached.count(state)) {
        this->attached[state]++;
      } else {
        this->attached[state] = 1;
      }
    }

    void LuaDebugHookDispatcher::detach(lua_State *state) {
      if (this->attached.count(state)) {
        this->attached[state]--;
        if (this->attached[state] > 0) {
          return;
        } else {
          this->attached.erase(state);
        }
      }
      if (this->callHooks.count(state)) {
        this->detachSet(this->callHooks[state]);
        this->callHooks.erase(state);
      }
      if (this->returnHooks.count(state)) {
        this->detachSet(this->returnHooks[state]);
        this->returnHooks.erase(state);
      }
      if (this->lineHooks.count(state)) {
        this->detachSet(this->lineHooks[state]);
        this->lineHooks.erase(state);
      }
      if (this->countHooks.count(state)) {
        this->detachSet(this->countHooks[state]);
        this->countHooks.erase(state);
      }
    }

    void LuaDebugHookDispatcher::bindState(std::map<lua_State *, std::set<std::size_t>> &map, lua_State *state, lua_Hook hook, int mask) {
      if (map.count(state) == 0) {
        map[state] = std::set<std::size_t>();
        lua_sethook(state, hook, mask, 1);
      }
    }

    void LuaDebugHookDispatcher::unbindState(std::map<lua_State *, std::set<std::size_t>> &map, lua_State *state, lua_Hook hook) {
      if (map[state].empty()) {
        lua_sethook(state, hook, 0, 0);
        map.erase(state);
      }
    }

    void LuaDebugHookDispatcher::callHook(lua_State *state, lua_Debug *debug) {
      if (LuaDebugHookDispatcher::getGlobal().callHooks.count(state)) {
        auto hooks = LuaDebugHookDispatcher::getGlobal().callHooks[state];
        for (std::size_t id : hooks) {
          LuaDebugHookDispatcher::getGlobal().hooks[id].second(state, debug);
        }
      }
    }

    void LuaDebugHookDispatcher::returnHook(lua_State *state, lua_Debug *debug) {
      if (LuaDebugHookDispatcher::getGlobal().returnHooks.count(state)) {
        auto hooks = LuaDebugHookDispatcher::getGlobal().returnHooks[state];
        for (std::size_t id : hooks) {
          LuaDebugHookDispatcher::getGlobal().hooks[id].second(state, debug);
        }
      }
    }

    void LuaDebugHookDispatcher::lineHook(lua_State *state, lua_Debug *debug) {
      if (LuaDebugHookDispatcher::getGlobal().lineHooks.count(state)) {
        auto hooks = LuaDebugHookDispatcher::getGlobal().lineHooks[state];
        for (std::size_t id : hooks) {
          LuaDebugHookDispatcher::getGlobal().hooks[id].second(state, debug);
        }
      }
    }

    void LuaDebugHookDispatcher::countHook(lua_State *state, lua_Debug *debug) {
      if (LuaDebugHookDispatcher::getGlobal().countHooks.count(state)) {
        auto hooks = LuaDebugHookDispatcher::getGlobal().countHooks[state];
        for (std::size_t id : hooks) {
          if (--LuaDebugHookDispatcher::getGlobal().counter[id].first == 0) {
            LuaDebugHookDispatcher::getGlobal().counter[id].first = LuaDebugHookDispatcher::getGlobal().counter[id].second;
            LuaDebugHookDispatcher::getGlobal().hooks[id].second(state, debug);
          }
        }
      }
    }

    void LuaDebugHookDispatcher::detachSet(std::set<std::size_t> &set) {
      for (std::size_t id : set) {
        this->hooks.erase(id);
        if (this->counter.count(id)) {
          this->counter.erase(id);
        }
      }
    }

    LuaDebugHookDispatcher &LuaDebugHookDispatcher::getGlobal() {
      return LuaDebugHookDispatcher::singleton;
    }
  }
}

#endif