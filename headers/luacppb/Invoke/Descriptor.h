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

		static NativeMethodDescriptor<C, M> *pushDescriptor(lua_State *state) {
			Internal::LuaStack stack(state);
			NativeMethodDescriptor<C, M> *descriptor = stack.push<NativeMethodDescriptor<C, M>>();
			stack.pushTable();
			stack.push(&NativeMethodDescriptor<C, M>::gcDescriptor);
			stack.setField(-2, "__gc");
			stack.setMetatable(-2);
			return descriptor;
		}
	 private:
	 	static int gcDescriptor(lua_State *state) {
			Internal::LuaStack stack(state);
			NativeMethodDescriptor<C, M> *descriptor = stack.toUserData<NativeMethodDescriptor<C, M> *>(1);
			if (descriptor) {
				descriptor->~NativeMethodDescriptor();
				::operator delete(descriptor, descriptor);
			}
			return 0;
		 }
	};

	template <typename T>
	class NativeInvocableDescriptor {
	 public:
	 	NativeInvocableDescriptor(T &value) : invocable(value) {}
		T invocable;

    static NativeInvocableDescriptor<T> *pushDescriptor(lua_State *state) {
      Internal::LuaStack stack(state);
      NativeInvocableDescriptor<T> *descriptor = stack.push<NativeInvocableDescriptor<T>>();
      stack.pushTable();
      stack.push(&NativeInvocableDescriptor<T>::gcDescriptor);
      stack.setField(-2, "__gc");
      stack.setMetatable(-2);
      return descriptor;
    }
   private:
    static int gcDescriptor(lua_State *state) {
      Internal::LuaStack stack(state);
      NativeInvocableDescriptor<T> *descriptor = stack.toUserData<NativeInvocableDescriptor<T> *>(1);
      if (descriptor) {
        descriptor->~NativeInvocableDescriptor();
        ::operator delete(descriptor, descriptor);
      }
      return 0;
    }
	};
}

#endif