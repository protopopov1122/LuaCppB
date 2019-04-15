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

#ifndef LUACPPB_CORE_DEBUG_H_
#define LUACPPB_CORE_DEBUG_H_

#include "luacppb/Base.h"
#include "luacppb/Core/Runtime.h"
#include "luacppb/Core/Stack.h"
#include <string>
#include <optional>
#include <memory>

namespace LuaCppB {

  namespace Internal {

    template <typename ... T>
    struct LuaDebugFunction_Impl {};

    template <typename F>
    struct LuaDebugFunction_Impl<F> {
      static void getWhat(std::string &line) {}
    };

    template <typename F, typename A, typename ... B>
    struct LuaDebugFunction_Impl<F, A, B...> {
      static void getWhat(std::string &line, A a, B... b) {
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
    };
  }

  class LuaDebugBaseFrame {
   public:
    enum class Function {
      Name,
      Source,
      Line,
      Params
    };

    LuaDebugBaseFrame(lua_State *, LuaCppRuntime &, lua_Debug *);
    LuaDebugBaseFrame(lua_State *, LuaCppRuntime &, int = 0);

    int getEvent() const;
    std::string getName() const;
    std::string getNameWhat() const;
    std::string getWhat() const;
    std::string getSource() const;
    std::string getShortSource() const;
    int getCurrentLine() const;
    int getLineDefined() const;
    int getLastLineDefined() const;
    unsigned char getUpvalues() const;
    unsigned char getParameters() const;
    bool isVarArg() const;
    bool isTailCall() const;

   protected:
    lua_State *state;
    LuaCppRuntime &runtime;
    lua_Debug debug;
  };

  template <typename Reference, typename ReferenceInternal>
  class LuaAbstractDebugFrame : public LuaDebugBaseFrame {
   public:
    struct Variable {
      std::string key;
      Reference value;
    };

    using LuaDebugBaseFrame::LuaDebugBaseFrame;
    using UpvalueId = void *;

    template <typename ... T>
    void getFunctionInfo(Reference ref, T ... args) {
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

    template <typename ... T>
    void getCurrentFunctionInfo(T ... args) {
      std::string what;
      Internal::LuaDebugFunction_Impl<Function, T...>::getWhat(what, args...);
      lua_getinfo(this->state, what.c_str(), &this->debug);
    }

    std::optional<Variable> getLocal(int index) {
      const char *key = lua_getlocal(this->state, &this->debug, index);
      if (key == nullptr) {
        return std::optional<Variable>();
      } else {
        Reference value(this->state, std::make_unique<ReferenceInternal>(this->state, this->runtime, -1));
        lua_pop(this->state, 1);
        return Variable { std::string(key), value };
      }
    }

    std::optional<std::string> getLocal(Reference ref, int index) {
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

    std::optional<Variable> getUpvalue(Reference ref, int index) {
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

    UpvalueId getUpvalueId(Reference ref, int index) {
      UpvalueId id = nullptr;
      ref.getReference().putOnTop([&](lua_State *state) {
        id = lua_upvalueid(state, -1, index);
      });
      return id;
    }
  };
}

#endif