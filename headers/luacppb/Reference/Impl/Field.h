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

#ifndef LUACPPB_REFERENCE_IMPL_FIELD_H_
#define LUACPPB_REFERENCE_IMPL_FIELD_H_

#include "luacppb/Reference/Field.h"

namespace LuaCppB {
  
  template <typename T>
  typename std::enable_if<Internal::LuaValueWrapper<T>::Conversible, LuaReferenceHandle>::type LuaReferenceHandle::operator[](T index) {
    if (this->ref) { // lgtm [cpp/empty-block]
      if constexpr (std::is_pointer<T>::value && std::is_same<char, typename std::decay<typename std::remove_pointer<T>::type>::type>::value) {
        std::string idx(index);
        return LuaReferenceHandle(this->state, std::make_unique<Internal::LuaStringField>(*this, this->ref->getRuntime(), idx));
      } else {
        return LuaReferenceHandle(this->state, std::make_unique<Internal::LuaTableField>(*this, this->ref->getRuntime(), Internal::LuaValueWrapper<T>::wrap(index)));
      }
    } else {
      return LuaReferenceHandle();
    }
  }
}

#endif