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

#include "luacppb/Core/Loader.h"
#include "luacppb/Reference/Primary.h"
#include <iostream>

namespace LuaCppB {

  struct LuaImageChunk {
    const LuaLoader::Image &image;
    std::size_t position;
  };

  constexpr std::size_t BUF_SIZE = 128;

  struct LuaTextChunk {
    std::istream &input;
    char buffer[BUF_SIZE];
  };

  static int LuaLoader_dump(lua_State *state, const void *data, std::size_t sz, void *udata) {
    const std::byte *bytes = reinterpret_cast<const std::byte *>(data);
    LuaLoader::Image chunk(bytes, bytes + sz);
    LuaLoader::Image &img = *reinterpret_cast<LuaLoader::Image *>(udata);
    img.insert(img.end(), chunk.begin(), chunk.end());
    return 0;
  }

  static const char *LuaLoader_load(lua_State *state, void *udata, std::size_t *sz) {
    LuaImageChunk &chunk = *reinterpret_cast<LuaImageChunk *>(udata);
    if (chunk.position < chunk.image.size()) {
      *sz = chunk.image.size() - chunk.position;
      std::size_t pos = chunk.position;
      chunk.position = chunk.image.size();
      return reinterpret_cast<const char *>(chunk.image.data() + pos);
    } else {
      *sz = 0;
      return nullptr;
    }
  }

  static const char *LuaLoader_loadText(lua_State *state, void *udata, std::size_t *sz) {
    LuaTextChunk &text = *reinterpret_cast<LuaTextChunk *>(udata);
    if (text.input.good()) {
      *sz = text.input.readsome(text.buffer, BUF_SIZE);
      return text.buffer;
    } else {
      *sz = 0;
      return nullptr;
    }
  }

  LuaLoader::LuaLoader(LuaState &state)
    : state(state) {}

  std::optional<LuaLoader::Image> LuaLoader::dump(const LuaReferenceHandle &ref, bool strip) {
    if (!ref.valid() || ref.getType() != LuaType::Function) {
      return std::optional<Image>();
    }
    std::optional<Image> img;
    ref.getReference().putOnTop([&](lua_State *state) {
      if (lua_iscfunction(state, -1)) {
        return;
      }
      Image image;
#ifndef LUACPPB_INTERNAL_COMPAT_502
      int res = lua_dump(state, LuaLoader_dump, reinterpret_cast<void *>(&image), static_cast<int>(strip));
#else
      int res = lua_dump(state, LuaLoader_dump, reinterpret_cast<void *>(&image));
#endif
      if (res == 0) {
        img = std::move(image);
      }
    });
    return img;
  }

  std::optional<LuaReferenceHandle> LuaLoader::load(const Image &img, const std::string &name) {
    if (!this->state.isValid()) {
      return std::optional<LuaReferenceHandle>();
    }
    LuaImageChunk chunk { img, 0 };
#ifndef LUACPPB_INTERNAL_COMPAT_501
    int res = lua_load(this->state.getState(), LuaLoader_load, reinterpret_cast<void *>(&chunk), name.c_str(), nullptr);
#else
    int res = lua_load(this->state.getState(), LuaLoader_load, reinterpret_cast<void *>(&chunk), name.c_str());
#endif
    switch (res) {
      case LUA_OK: {
        LuaReferenceHandle handle(this->state.getState(), std::make_unique<Internal::LuaRegistryReference>(this->state.getState(), this->state, -1));
        lua_pop(this->state.getState(), 1);
        return std::optional<LuaReferenceHandle>(handle);
      } break;
      default:
        break;
    }
    return std::optional<LuaReferenceHandle>();
  }

  std::optional<LuaReferenceHandle> LuaLoader::load(std::istream &is, const std::string &name) {
    if (!this->state.isValid() || !is.good()) {
      return std::optional<LuaReferenceHandle>();
    }
    LuaTextChunk text { is, {} };
#ifndef LUACPPB_INTERNAL_COMPAT_501
    int res = lua_load(this->state.getState(), LuaLoader_loadText, reinterpret_cast<void *>(&text), name.c_str(), nullptr);
#else
    int res = lua_load(this->state.getState(), LuaLoader_loadText, reinterpret_cast<void *>(&text), name.c_str());
#endif
    switch (res) {
      case LUA_OK: {
        LuaReferenceHandle handle(this->state.getState(), std::make_unique<Internal::LuaRegistryReference>(this->state.getState(), this->state, -1));
        lua_pop(this->state.getState(), 1);
        return std::optional<LuaReferenceHandle>(handle);
      } break;
      default:
        break;
    }
    return std::optional<LuaReferenceHandle>();
  }
}