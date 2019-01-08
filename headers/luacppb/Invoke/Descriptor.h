#ifndef LUACPPB_INVOKE_DESCRIPTOR_H_
#define LUACPPB_INVOKE_DESCRIPTOR_H_

#include "luacppb/Base.h"
#include "luacppb/Core/Stack.h"

namespace LuaCppB::Internal {

	template <typename C, typename M>
	class NativeMethodDescriptor {
	 public:
		C *object;
		M method;

		static NativeMethodDescriptor<C, M> *pushDescriptor(lua_State *);
	 private:
	 	static int gcDescriptor(lua_State *);
	};

	template <typename T>
	class NativeInvocableDescriptor {
	 public:
	 	NativeInvocableDescriptor(T &);
		T invocable;

    static NativeInvocableDescriptor<T> *pushDescriptor(lua_State *);
   private:
    static int gcDescriptor(lua_State *);
	};
}

#include "luacppb/Invoke/Impl/Descriptor.h"

#endif