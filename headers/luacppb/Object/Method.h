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

#ifndef LUACPPB_OBJECT_METHOD_H_
#define LUACPPB_OBJECT_METHOD_H_

#include "luacppb/Base.h"
#include "luacppb/Value/Native.h"
#include "luacppb/Object/Wrapper.h"
#include "luacppb/Invoke/ArgRet.h"
#include "luacppb/Invoke/Method.h"
#include "luacppb/Core/Stack.h"
#include "luacppb/Core/Runtime.h"
#include <optional>

namespace LuaCppB::Internal {

	template <typename C, typename M, typename R, typename ... A>
	class LuaCppObjectMethodCall : public LuaData {
	 public:
		LuaCppObjectMethodCall(M, const std::string &, LuaCppRuntime &);

		void push(lua_State *) const override;
	 private:
	 	static int call(C *, M, LuaCppRuntime &, lua_State *);
		static int class_method_closure(lua_State *);
		static LuaCppObjectWrapper<C> *checkUserData(const std::string &, lua_State *, int = -1);

		M method;
		std::string className;
		LuaCppRuntime &runtime;
	};
}

#include "luacppb/Object/Impl/Method.h"

#endif
