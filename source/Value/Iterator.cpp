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

#include "luacppb/Value/Iterator.h"
#include "luacppb/Reference/Handle.h"
#include "luacppb/Reference/Primary.h"

namespace LuaCppB {
  const TableIterator TableIterator::End;

  TableIterator::TableIterator()
    : state(nullptr), table(), index(std::make_pair(LuaReferenceHandle(), LuaReferenceHandle())) {}

  TableIterator::TableIterator(lua_State *state, LuaReferenceHandle table)
    : state(state), table(std::move(table)) {
    if (state) {
      Internal::LuaStack stack(state);
      stack.push();
      LuaReferenceHandle nil(state, std::make_unique<Internal::LuaRegistryReference>(state, this->table.getRuntime(), -1));
      this->index = std::make_pair(nil, nil);
      stack.pop();
      this->next();
    }
  }

  TableIterator::TableIterator(const TableIterator &it)
    : state(it.state), table(it.table), index(it.index) {}
  
  TableIterator::TableIterator(TableIterator &&it)
    : state(it.state), table(std::move(it.table)), index(std::move(it.index)) {
    it.state = nullptr;
  }

  bool TableIterator::valid() const {
    return this->state != nullptr && this->table.valid();
  }

  TableIterator::Entry TableIterator::operator*() const {
    return std::make_pair(*this->index.first, *this->index.second);
  }

  TableIterator &TableIterator::operator++() {
    this->next();
    return *this;
  }

  TableIterator TableIterator::operator++(int) {
    TableIterator it(*this);
    this->operator++();
    return it;
  }

  bool TableIterator::operator==(const TableIterator &it) const {
    if (this->state && it.state) {
      return this->state == it.state &&
        this->table == it.table &&
        this->index.first == it.index.first;
    } else {
      return this->state == it.state;
    }
  }

  bool TableIterator::operator!=(const TableIterator &it) const {
    if (this->state) {
      return this->state != it.state ||
        this->table != it.table ||
        this->index.first != it.index.first;
    } else {
      return this->state != it.state;
    }
  }

  void TableIterator::next() {
    if (this->valid()) {
      Internal::LuaStack stack(this->state);
      (*this->table).push(this->state);
      (*this->index.first).push(this->state);
      int res = lua_next(this->state, -2);
      if (res) {
        LuaReferenceHandle key(this->state, std::make_unique<Internal::LuaRegistryReference>(this->state, this->table.getRuntime(), -2));
        LuaReferenceHandle value(this->state, std::make_unique<Internal::LuaRegistryReference>(this->state, this->table.getRuntime(), -1));
        this->index = std::make_pair(key, value);
        stack.pop(3);
      } else {
        stack.pop(1);
        this->state = nullptr;
        this->index = std::make_pair(LuaReferenceHandle(), LuaReferenceHandle());
      }
    }
  }
}