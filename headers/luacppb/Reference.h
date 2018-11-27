#ifndef LUACPPB_REFERENCE_H_
#define LUACPPB_REFERENCE_H_

#include "luacppb/Base.h"
#include "luacppb/Global.h"
#include "luacppb/Stack.h"
#include "luacppb/Value.h"
#include "luacppb/Function.h"
#include <type_traits>
#include <string>
#include <functional>
#include <memory>
#include <iostream>

namespace LuaCppB {

	class LuaReferenceHandle;

	class LuaReference {
	 public:
		virtual ~LuaReference() = default;
		bool set(LuaData &);

		virtual void putOnTop(std::function<void (lua_State *)>) = 0;
		virtual bool setValue(std::function<void (lua_State *)>) = 0;

		template <typename T>
		typename std::enable_if<std::is_same<T, LuaValue>::value, T>::type get() {
			std::optional<LuaValue> value;
			this->putOnTop([&](lua_State *state) {
				value = LuaValue::peek(state);
			});
			return value.value_or(LuaValue());
		}

		template <typename T>
		typename std::enable_if<!std::is_same<T, LuaValue>::value, T>::type get() {
			return this->get<LuaValue>().get<T>();
		}

		template <typename T>
		typename std::enable_if<std::is_base_of<LuaData, T>::value, bool>::type set(T value) {
			return this->setValue([&value](lua_State *state) {
				value.push(state);
			});
		}

		template <typename T>
		typename std::enable_if<!std::is_base_of<LuaData, T>::value, bool>::type set(T value) {
			LuaValue val = LuaValue::create<T>(value);
			return this->setValue([&val](lua_State *state) {
				val.push(state);
			});
		}
	};

	class LuaGlobalVariable : public LuaReference {
	 public:
		LuaGlobalVariable(const LuaGlobalScope &scope, const std::string &name) : scope(scope), name(name) {}
	
		void putOnTop(std::function<void (lua_State *)>) override;
		bool setValue(std::function<void (lua_State *)>) override;
	 private:
		LuaGlobalScope scope;
		std::string name;
	};

	class LuaStackReference : public LuaReference {
	 public:
		LuaStackReference(const LuaStack &stack, lua_Integer index) : stack(stack), index(index) {}

		void putOnTop(std::function<void (lua_State *)>) override;
		bool setValue(std::function<void (lua_State *)> gen) override;
	 private:
		LuaStack stack;
		lua_Integer index;
	};

	class LuaReferenceHandle {
	 public:
		LuaReferenceHandle(std::shared_ptr<LuaReference> ref) : ref(ref) {}

		LuaReference &getReference();
		bool exists();
		LuaType getType();
		LuaReferenceHandle operator[](const std::string &);
		LuaReferenceHandle operator[](lua_Integer);
		LuaReferenceHandle &operator=(LuaData &);

		template <typename R, typename ... A>
		LuaReferenceHandle &operator=(R (*fn)(A...)) {
			CFunctionCall call(fn);
			this->ref->set(call);
			return *this;
		}

		template <typename T>
		LuaReferenceHandle &operator=(T value) {
			this->ref->set<T>(value);
			return *this;
		}

		template <typename T>
		operator T () {
			return this->ref->get<T>();
		}

		template <typename T>
		T get() {
			return this->ref->get<T>();
		}
	 private:
		std::shared_ptr<LuaReference> ref;
	};

	class LuaTableField : public LuaReference {
	 public:
		LuaTableField(LuaReferenceHandle ref, const std::string &name) : ref(ref), name(name) {}
		
		void putOnTop(std::function<void (lua_State *)>) override;
		bool setValue(std::function<void (lua_State *)>) override;
	 private:
		LuaReferenceHandle ref;
		std::string name;
	};

	class LuaArrayField : public LuaReference {
	 public:
		LuaArrayField(LuaReferenceHandle ref, lua_Integer index) : ref(ref), index(index) {}

		void putOnTop(std::function<void (lua_State *)>) override;
		bool setValue(std::function<void (lua_State *)>) override;
	 private:
		LuaReferenceHandle ref;
		lua_Integer index;
	};
}

#endif
