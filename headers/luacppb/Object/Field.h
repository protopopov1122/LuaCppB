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

#ifndef LUACPPB_OBJECT_FIELD_H_
#define LUACPPB_OBJECT_FIELD_H_

#include "luacppb/Base.h"
#include "luacppb/Core/Runtime.h"
#include "luacppb/Value/Native.h"
#include <memory>

namespace LuaCppB::Internal {

	class LuaCppObjectFieldPusher {
	 public:
		virtual ~LuaCppObjectFieldPusher() = default;
		virtual void push(lua_State *, void *) const = 0;
	};

	template <typename C, typename B, typename T>
	class LuaCppObjectFieldHandle : public LuaCppObjectFieldPusher {
	 public:
		LuaCppObjectFieldHandle(T C::*, LuaCppRuntime &);

		void push(lua_State *, void *) const override;
	 private:
		T C::*field;
		LuaCppRuntime &runtime;
	};

  class LuaCppObjectFieldController {
   public:
    LuaCppObjectFieldController(const std::map<std::string, std::shared_ptr<LuaCppObjectFieldPusher>> &);

    void get(lua_State *, void *, const std::string &);
    static void push(lua_State *, const std::map<std::string, std::shared_ptr<LuaCppObjectFieldPusher>> &);
    static void pushFunction(lua_State *, const std::map<std::string, std::shared_ptr<LuaCppObjectFieldPusher>> &);
   private:
    static int indexFunction(lua_State *);
    static int gcObject(lua_State *);
    std::map<std::string, std::shared_ptr<LuaCppObjectFieldPusher>> fields;
  };
}

#endif
