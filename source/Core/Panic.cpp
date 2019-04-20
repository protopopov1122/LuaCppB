#include "luacppb/Core/Panic.h"
#include "luacppb/Core/Stack.h"

namespace LuaCppB::Internal {

  LuaPanicDispatcher LuaPanicDispatcher::singleton;

  lua_CFunction LuaPanicDispatcher::attach(lua_State *state, Handler handler) {
    this->handlers[state] = handler;
    return lua_atpanic(state, &LuaPanicDispatcher::atpanic);
  }

  void LuaPanicDispatcher::detach(lua_State *state) {
    if (this->handlers.count(state)) {
      this->handlers.erase(state);
    }
  }

  LuaPanicDispatcher &LuaPanicDispatcher::getGlobal() {
    return LuaPanicDispatcher::singleton;
  }

  int LuaPanicDispatcher::atpanic(lua_State *state) {
      LuaStack stack(state);
#ifndef LUACPPB_EMULATED_MAINTHREAD
      stack.getIndex<true>(LUA_REGISTRYINDEX, LUA_RIDX_MAINTHREAD);
#else
      stack.push(std::string(LUACPPB_RIDX_MAINTHREAD));
      stack.getField<true>(LUA_REGISTRYINDEX);
#endif
    state = stack.toThread();
    stack.pop();
    auto &dispatcher = LuaPanicDispatcher::getGlobal();
    if (dispatcher.handlers.count(state)) {
      return dispatcher.handlers[state](state);
    } else {
      return 0;
    }
  }

  LuaPanicUnbind::LuaPanicUnbind(LuaPanicDispatcher &dispatcher)
    : dispatcher(dispatcher), state(nullptr) {}
  
  LuaPanicUnbind::LuaPanicUnbind(LuaPanicDispatcher &dispatcher, lua_State *state)
    : dispatcher(dispatcher), state(state) {}
    
  LuaPanicUnbind::LuaPanicUnbind(LuaPanicUnbind &&unbind)
    : dispatcher(unbind.dispatcher), state(unbind.state) {
    unbind.state = nullptr;
  }
  
  LuaPanicUnbind &LuaPanicUnbind::operator=(LuaPanicUnbind &&unbind) {
    this->state = unbind.state;
    unbind.state = nullptr;
    return *this;
  }

  LuaPanicUnbind::~LuaPanicUnbind() {
    if (this->state) {
      this->dispatcher.detach(this->state);
    }
  }
}