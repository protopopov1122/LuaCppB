#ifndef LUACPPB_OBJECT_WRAPPER_H_
#define LUACPPB_OBJECT_WRAPPER_H_

#include "luacppb/Base.h"
#include <memory>
#include <variant>

namespace LuaCppB {

	template <typename C>
	class LuaCppObjectWrapper {
		using Raw = C *;
		using Unique = std::unique_ptr<C>;
		using Shared = std::shared_ptr<C>;
	 public:
	 	LuaCppObjectWrapper(Raw obj) : object(obj) {}
		LuaCppObjectWrapper(C &obj) : object(&obj) {}
	 	LuaCppObjectWrapper() {
		  this->object = std::unique_ptr<C>(reinterpret_cast<C *>(::operator new(sizeof(C))));
		}
		LuaCppObjectWrapper(Unique obj) : object(std::move(obj)) {}
		LuaCppObjectWrapper(Shared obj) : object(obj) {}
		~LuaCppObjectWrapper() {
			this->object = nullptr;
		}

		C *get() {
			switch (this->object.index()) {
				case 0:
				  return std::get<Raw>(object);
				case 1:
					return std::get<Unique>(object).get();
				case 2:
					return std::get<Shared>(object).get();
				default:
					return nullptr;
			}
		}
	 private:
		std::variant<Raw, Unique, Shared> object;
	};
}

#endif