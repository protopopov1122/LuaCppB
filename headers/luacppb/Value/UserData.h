#ifndef LUACPPB_VALUE_USERDATA_H_
#define LUACPPB_VALUE_USERDATA_H_

#include "luacppb/Reference/Registry.h"
#include "luacppb/Value/Native.h"
#include "luacppb/Invoke/Native.h"
#include <sstream>
#include <atomic>
#include <map>

namespace LuaCppB {

  namespace Internal {

    template <typename T, typename F, typename ... A>
    class CustomUserDataCall : public LuaData {
      using R = typename std::invoke_result<F, T &, A...>::type;
    public:
      CustomUserDataCall(F &&inv, LuaCppRuntime &runtime) : invocable(inv), runtime(runtime) {}

      void push(lua_State *state) const {
        Internal::LuaStack stack(state);
        NativeInvocableDescriptor<F> *descriptor = stack.push<NativeInvocableDescriptor<F>>();
        new(descriptor) NativeInvocableDescriptor(this->invocable);
        stack.push(&this->runtime);
        stack.push(&CustomUserDataCall<T, F, A...>::userdata_closure, 2);
      }
    private:
      static int call(F &invocable, LuaCppRuntime &runtime, lua_State *state) {
        std::array<LuaValue, sizeof...(A)> wrappedArgs;
        WrappedFunctionArguments<2, A...>::get(state, wrappedArgs);
        std::tuple<A...> args = Internal::NativeFunctionArgumentsTuple<2, A...>::value(state, runtime, wrappedArgs);
        T &value = *static_cast<T *>(lua_touserdata(state, 1));
        std::tuple<T &, A...> fullArgs = std::tuple_cat(std::forward_as_tuple(value), args);
        if constexpr (std::is_void<R>::value) {
          std::apply(invocable, fullArgs);
          return 0;
        } else {
          return Internal::NativeFunctionResult<LuaNativeValue, R>::set(state, runtime, std::apply(invocable, fullArgs));
        }
      }

      static int userdata_closure(lua_State *state) {
        try {
          Internal::LuaStack stack(state);
          NativeInvocableDescriptor<F> *descriptor = stack.toUserData<NativeInvocableDescriptor<F> *>(lua_upvalueindex(1));
          LuaCppRuntime &runtime = *stack.toPointer<LuaCppRuntime *>(lua_upvalueindex(2));
          return CustomUserDataCall<T, F, A...>::call(descriptor->invocable, runtime, state);
        } catch (std::exception &ex) {
          return luacpp_handle_exception(state, std::current_exception());
        }
      }

      F invocable;
      LuaCppRuntime &runtime;
    };

    template <typename C, typename T>
    struct CustomUserDataCallBuilder : public CustomUserDataCallBuilder<C, decltype(&T::operator())> {};

    template <typename T, typename R, typename ... A>
    struct CustomUserDataCallBuilder<T, R(T &, A...)> {
      using FunctionType = std::function<R(T &, A...)>;
      using Type = CustomUserDataCall<T, FunctionType, A...>;
      static std::shared_ptr<Type> create(FunctionType && f, LuaCppRuntime &runtime) {
        return std::make_shared<Type>(std::forward<FunctionType>(f), runtime);
      }
    };

    template<typename T, typename R, typename ... A>
    struct CustomUserDataCallBuilder<T, R(*)(T &, A...)> {
      using FunctionType = std::function<R(T &, A...)>;
      using Type = CustomUserDataCall<T, FunctionType, A...>;
      static std::shared_ptr<Type> create(FunctionType && f, LuaCppRuntime &runtime) {
        return std::make_shared<Type>(std::forward<FunctionType>(f), runtime);
      }
    };

    template<typename T, typename C, typename R, typename ... A>
    struct CustomUserDataCallBuilder<T, R (C::*)(T &, A...) const> {
      using FunctionType = std::function<R(T &, A...)>;
      using Type = CustomUserDataCall<T, FunctionType, A...>;
      static std::shared_ptr<Type> create(FunctionType && f, LuaCppRuntime &runtime) {
        return std::make_shared<Type>(std::forward<FunctionType>(f), runtime);
      }
    };
  }

  enum class LuaMetamethod {
    GC,
    Index,
    NewIndex
    // TODO Full list of metamethods
  };

  template <typename T>
  class CustomUserData : public LuaData {
   public:
    CustomUserData(T *pointer, Internal::LuaSharedRegistryHandle handle)
      : pointer(pointer), handle(handle) {}
    
    operator T*() {
      return this->get();
    }
    
    operator T&() {
      return *this->get();
    }

    T *get() {
      return this->pointer;
    }

	  void push(lua_State *state) const override {
      Internal::LuaStack stack(state);
      int ref = -1;
      this->handle.get([&](lua_State *handleState) {
        Internal::LuaStack handleStack(handleState);
        handleStack.copy(-1);
        ref = handleStack.ref();
      });
      stack.getIndex<true>(LUA_REGISTRYINDEX, ref);
      stack.unref(ref);
    }
   private:
    T *pointer;
    Internal::LuaSharedRegistryHandle handle;
  };

  template <typename T>
  class CustomUserDataClass {
   public:
    CustomUserDataClass(LuaCppRuntime &runtime)
      : runtime(runtime), constructor([](T &) {}) {
      this->className = LuaUserData::getCustomName<T>(CustomUserDataClass::nextIdentifier++);
    }

    void setDefaultConstructor(std::function<void(T &)> fn) {
      this->constructor = fn;
    }

    CustomUserData<T> create(lua_State *state, std::function<void(T &)> constructor) const {
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

    CustomUserData<T> create(lua_State *state) const {
      return this->create(state, this->constructor);
    }

    template <typename F>
    void bind(const std::string &key, F &&fn) {
      this->methods[key] = Internal::CustomUserDataCallBuilder<T, F>::create(std::forward<F>(fn), this->runtime);
    }

    template <typename F>
    bool bind(LuaMetamethod key, F &&fn) {
      if (CustomUserDataClass<T>::metamethods.count(key)) {
        this->bind(CustomUserDataClass<T>::metamethods[key], std::forward<F>(fn));
        return true;
      } else {
        return false;
      }
    }
   private:
    LuaCppRuntime &runtime;
    std::string className;
    std::function<void(T &)> constructor;
    std::map<std::string, std::shared_ptr<LuaData>> methods;

    static std::atomic<uint64_t> nextIdentifier;
    static std::map<LuaMetamethod, std::string> metamethods;
  };

  template <typename T>
  std::atomic<uint64_t> CustomUserDataClass<T>::nextIdentifier(0);

  template <typename T>
  std::map<LuaMetamethod, std::string> CustomUserDataClass<T>::metamethods = {
    { LuaMetamethod::GC, "__gc" },
    { LuaMetamethod::Index, "__index" },
    { LuaMetamethod::NewIndex, "__newindex" }
    // TODO Full list of metamethods
  };
}

#endif