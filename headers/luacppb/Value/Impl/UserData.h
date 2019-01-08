#ifndef LUACPPB_VALUE_IMPL_USERDATA_H_
#define LUACPPB_VALUE_IMPL_USERDATA_H_

#include "luacppb/Value/UserData.h"

namespace LuaCppB {

  namespace Internal {

    template <typename T, typename F, typename ... A>
    CustomUserDataCall<T, F, A...>::CustomUserDataCall(F &&inv, const std::string &className, LuaCppRuntime &runtime)
      : invocable(inv), className(className), runtime(runtime) {}
    
    template <typename T, typename F, typename ... A>
    void CustomUserDataCall<T, F, A...>::push(lua_State *state) const {
      Internal::LuaStack stack(state);
      NativeInvocableDescriptor<F> *descriptor = NativeInvocableDescriptor<F>::pushDescriptor(state);
      new(descriptor) NativeInvocableDescriptor(this->invocable);
      stack.push(this->className);
      stack.push(&this->runtime);
      stack.push(&CustomUserDataCall<T, F, A...>::userdata_closure, 3);
    }

    template <typename T, typename F, typename ... A>
    int CustomUserDataCall<T, F, A...>::call(F &invocable, const std::string &className, LuaCppRuntime &runtime, lua_State *state) {
      std::array<LuaValue, sizeof...(A)> wrappedArgs;
      WrappedFunctionArguments<2, A...>::get(state, wrappedArgs);
      std::tuple<A...> args = Internal::NativeFunctionArgumentsTuple<2, A...>::value(state, runtime, wrappedArgs);
      T &value = *static_cast<T *>(luaL_checkudata(state, 1, className.c_str()));
      std::tuple<T &, A...> fullArgs = std::tuple_cat(std::forward_as_tuple(value), args);
      if constexpr (std::is_void<R>::value) {
        std::apply(invocable, fullArgs);
        return 0;
      } else {
        return Internal::NativeFunctionResult<LuaNativeValue, R>::set(state, runtime, std::apply(invocable, fullArgs));
      }
    }

    template <typename T, typename F, typename ... A>
    int CustomUserDataCall<T, F, A...>::userdata_closure(lua_State *state) {
      try {
        Internal::LuaStack stack(state);
        NativeInvocableDescriptor<F> *descriptor = stack.toUserData<NativeInvocableDescriptor<F> *>(lua_upvalueindex(1));
        std::string className = stack.toString(lua_upvalueindex(2));
        LuaCppRuntime &runtime = *stack.toPointer<LuaCppRuntime *>(lua_upvalueindex(3));
        return CustomUserDataCall<T, F, A...>::call(descriptor->invocable, className, runtime, state);
      } catch (std::exception &ex) {
        return luacpp_handle_exception(state, std::current_exception());
      }
    }

    template <typename C, typename T>
    std::shared_ptr<typename CustomUserDataCallBuilder<C, T>::Type> CustomUserDataCallBuilder<C, T>::create(typename CustomUserDataCallBuilder<C, T>::FunctionType && f, const std::string &className, LuaCppRuntime &runtime) {
      return std::make_shared<Type>(std::forward<FunctionType>(f), className, runtime);
    }
  }

  template <typename T>
  CustomUserData<T>::CustomUserData(T *pointer, Internal::LuaSharedRegistryHandle handle)
    : pointer(pointer), handle(handle) {}
  
  template <typename T>
  CustomUserData<T>::operator T*() {
    return this->get();
  }
  
  template <typename T>
  CustomUserData<T>::operator T&() {
    return *this->get();
  }

  template <typename T>
  T *CustomUserData<T>::get() {
    return this->pointer;
  }

  template <typename T>
  void CustomUserData<T>::push(lua_State *state) const {
    this->handle.push(state);
  }

  template <typename T>
  CustomUserDataClass<T>::CustomUserDataClass(LuaCppRuntime &runtime)
    : runtime(runtime), constructor([](T &) {}) {
    this->className = LuaUserData::getCustomName<T>(CustomUserDataClass::nextIdentifier++);
  }

  template <typename T>
  void CustomUserDataClass<T>::setDefaultConstructor(std::function<void(T &)> &&fn) {
    this->constructor = fn;
  }

  template <typename T>
  template <typename S>
  CustomUserData<T> CustomUserDataClass<T>::create(S &env, const std::function<void(T &)> &&constructor) const {
    lua_State *state = env.getState();
    Internal::LuaStack stack(state);
    T *value = stack.push<T>();
    if (stack.metatable(this->className)) {
      for (auto it = this->methods.begin(); it != this->methods.end(); ++it) {
        it->second->push(state);
        stack.setField(-2, it->first);
      }
    }
    stack.setMetatable(-2);
    Internal::LuaSharedRegistryHandle handle(state, -1);
    stack.pop();
    constructor(*value);
    return CustomUserData<T>(value, handle);
  }

  template <typename T>
  template <typename S>
  CustomUserData<T> CustomUserDataClass<T>::create(S &env) const {
    return this->create(env, std::function(this->constructor));
  }

  template <typename T>
  template <typename F>
  void CustomUserDataClass<T>::bind(const std::string &key, F &&fn) {
    this->methods[key] = Internal::CustomUserDataCallBuilder<T, F>::create(std::forward<F>(fn), this->className, this->runtime);
  }

  template <typename T>
  template <typename F>
  bool CustomUserDataClass<T>::bind(LuaMetamethod key, F &&fn) {
    if (CustomUserDataClass<T>::metamethods.count(key)) {
      this->bind(CustomUserDataClass<T>::metamethods[key], std::forward<F>(fn));
      return true;
    } else {
      return false;
    }
  }

  template <typename T>
  std::atomic<uint64_t> CustomUserDataClass<T>::nextIdentifier(0);

  template <typename T>
  std::map<LuaMetamethod, std::string> CustomUserDataClass<T>::metamethods = {
    { LuaMetamethod::GC, "__gc" },
    { LuaMetamethod::Index, "__index" },
    { LuaMetamethod::NewIndex, "__newindex" },
    { LuaMetamethod::Call, "__call" },
    { LuaMetamethod::ToString, "__tostring" },
    { LuaMetamethod::Length, "__len" },
    { LuaMetamethod::Pairs, "__pairs" },
    { LuaMetamethod::IPairs, "__ipairs" },
    //Mathematical
    { LuaMetamethod::UnaryMinus, "__unm" },
    { LuaMetamethod::Add, "__add" },
    { LuaMetamethod::Subtract, "__sub" },
    { LuaMetamethod::Multiply, "__mul" },
    { LuaMetamethod::Divide, "__div" },
    { LuaMetamethod::FloorDivide, "__idiv" },
    { LuaMetamethod::Modulo, "__mod" },
    { LuaMetamethod::Power, "__pow" },
    { LuaMetamethod::Concat, "__concat" },
    // Bitwise
    { LuaMetamethod::BitwiseAnd, "__band" },
    { LuaMetamethod::BitwiseOr, "__bor" },
    { LuaMetamethod::BitwiseXor, "__bxor" },
    { LuaMetamethod::BitwiseNot, "__bnot" },
    { LuaMetamethod::ShiftLeft, "__shl" },
    { LuaMetamethod::ShiftRight, "__shr" },
    // Equivalence
    { LuaMetamethod::Equals, "__eq" },
    { LuaMetamethod::LessThan, "__lt" },
    { LuaMetamethod::LessOrEqual, "__le" }
  };
}

#endif