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

#ifndef LUACPPB_REFERENCE_PRIMARY_H_
#define LUACPPB_REFERENCE_PRIMARY_H_

#include "luacppb/Reference/Base.h"
#include "luacppb/Object/Registry.h"
#include "luacppb/Core/Error.h"

namespace LuaCppB::Internal {

	class LuaGlobalVariable : public LuaReference {
	 public:
		LuaGlobalVariable(LuaState &, const std::string &);
	
		bool putOnTop(std::function<void (lua_State *)>) override;
		bool setValue(std::function<void (lua_State *)>) override;
		lua_State *putOnTop() override;
		std::unique_ptr<LuaReference> clone(LuaCppRuntime &) override;

	 private:
		lua_State *state;
		std::string name;
	};

	class LuaStackReference : public LuaReference {
	 public:
		LuaStackReference(LuaState &, int);
		LuaStackReference(const LuaStackReference &) = default;

		bool putOnTop(std::function<void (lua_State *)>) override;
		bool setValue(std::function<void (lua_State *)> gen) override;
		lua_State *putOnTop() override;
		std::unique_ptr<LuaReference> clone(LuaCppRuntime &) override;

	 private:
		lua_State *state;
		int index;
	};

	class LuaRegistryReference : public LuaReference {
	 public:
		LuaRegistryReference(lua_State *, LuaCppRuntime &, int = -1);
		LuaRegistryReference(LuaCppRuntime &, Internal::LuaSharedRegistryHandle &);

		bool putOnTop(std::function<void (lua_State *)>) override;
		bool setValue(std::function<void (lua_State *)>) override;
		lua_State *putOnTop() override;
		std::unique_ptr<LuaReference> clone(LuaCppRuntime &) override;

	 protected:
		LuaValue toValue() override;
	 private:
	 	Internal::LuaSharedRegistryHandle handle;
	};
}

#endif
