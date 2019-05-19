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

#ifndef LUACPPB_REFERENCE_FAST_PATH_H_
#define LUACPPB_REFERENCE_FAST_PATH_H_

#include "luacppb/Base.h"

#ifdef LUACPPB_FAST_REFERENCE_SUPPORT

#include "luacppb/Reference/Handle.h"
#include "luacppb/Reference/Primary.h"
#include "luacppb/Value/Iterator.h"
#include "luacppb/Value/Native.h"

namespace LuaCppB {

  namespace Internal {

    template <typename R>
    class LuaFastRef {
     public:
      operator LuaReferenceHandle() const;
      Internal::LuaReference &getReference() const;
      LuaReferenceHandle persistent() const;
      TableIterator begin() const;
      const TableIterator &end() const;
      LuaReferenceHandle getMetatable() const;
      void setMetatable(LuaData &d);
      void setMetatable(LuaData &&d);

      template <typename T>
      typename std::enable_if<Internal::LuaValueWrapper<T>::Conversible, LuaReferenceHandle>::type operator[](T);

      template <typename T, typename Type = typename std::enable_if<(!std::is_class<T>::value || Internal::LuaReferenceGetSpecialCase<T>::value) && !std::is_same<LuaReferenceHandle, typename std::decay<T>::type>::value, T>::type>
      operator T ();

      template <typename T, typename Type = typename std::enable_if<std::is_class<T>::value && !Internal::LuaReferenceGetSpecialCase<T>::value && !std::is_same<LuaReferenceHandle, typename std::decay<T>::type>::value, T>::type>
      operator T& ();

      template <typename ... A>
      Internal::LuaFunctionCallResult operator()(A &&...) const;
    };

    class LuaVariableRef : public LuaFastRef<LuaVariableRef> {
     public:
      LuaVariableRef(LuaState &, const std::string &);
      LuaVariableRef() = delete;
      LuaVariableRef(const LuaVariableRef &);
      LuaVariableRef(LuaVariableRef &&);

      LuaVariableRef &operator=(const LuaVariableRef &);
      LuaVariableRef &operator=(LuaVariableRef &&);
    
      LuaReferenceHandle &getHandle() const;
      LuaCppRuntime &getRuntime() const;
      bool valid() const;
      bool exists() const;

      LuaType getType() const;
      LuaValue operator*() const;

      template <typename T>
      typename std::enable_if<!std::is_same<T, LuaVariableRef>::value, LuaVariableRef>::type &operator=(T &);

      template <typename T>
      typename std::enable_if<!std::is_same<T, LuaVariableRef>::value, LuaVariableRef>::type &operator=(T &&);

      template <typename T>
      typename std::enable_if<!Internal::is_instantiation<std::function, T>::value && !std::is_same<LuaReferenceHandle, typename std::decay<T>::type>::value, T>::type get() const;
     private:
      LuaState &state;
      std::string name;
      LuaReferenceHandle handle;
    };

    bool operator==(const LuaVariableRef &, const LuaReferenceHandle &);
    bool operator!=(const LuaVariableRef &, const LuaReferenceHandle &);
  }
}

#endif

#endif