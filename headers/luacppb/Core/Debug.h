#ifndef LUACPPB_CORE_DEBUG_H_
#define LUACPPB_CORE_DEBUG_H_

#include "luacppb/Base.h"
#include "luacppb/Core/Stack.h"
#include <string>

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

  class LuaDebugFrame {
   public:
    enum class Function {
      Name,
      Source,
      Line,
      Params
    };

    LuaDebugFrame(lua_State *, int = 0);

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

    template <typename R, typename ... T>
    void getFunctionInfo(R ref, T ... args) {
      std::string line = ">";
      Internal::LuaDebugFunction_Impl<Function, T...>::getWhat(line, args...);
      ref.getReference().putOnTop([&](lua_State *state) {
        Internal::LuaStack stack(state);
        stack.copy(-1);
        lua_getinfo(state, line.c_str(), &this->debug);
      });
    }

    template <typename S, typename ... T>
    void getCurrentFunctionInfo(S &state, T ... args) {
      std::string line;
      Internal::LuaDebugFunction_Impl<Function, T...>::getWhat(line, args...);
      lua_getinfo(state.getState(), line.c_str(), &this->debug);
    }
   private:
    lua_Debug debug;
  };
}

#endif