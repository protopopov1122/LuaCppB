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
      static void getWhat(std::string &);
    };

    template <typename F, typename A, typename ... B>
    struct LuaDebugFunction_Impl<F, A, B...> {
      static void getWhat(std::string &, A, B...);
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
    void getFunctionInfo(Reference, T...);
    
    template <typename ... T>
    void getCurrentFunctionInfo(T...);

    std::optional<Variable> getLocal(int);
    std::optional<std::string> getLocal(Reference, int);
    std::optional<Variable> getUpvalue(Reference, int);
    UpvalueId getUpvalueId(Reference, int);
    bool setLocal(int, Reference);
    bool setUpvalue(Reference, int, Reference);
    void joinUpvalues(Reference, int, Reference, int);
  };
}

#include "luacppb/Core/Impl/Debug.h"

#endif