#ifndef LUACPPB_REFERENCE_H_
#define LUACPPB_REFERENCE_H_

#include "luacppb/Base.h"
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
		lua_State *getState();

		virtual void putOnTop(std::function<void (lua_State *)>) = 0;
		virtual void setValue(std::function<void (lua_State *)>) = 0;

		template <typename T>
		typename std::enable_if<std::is_same<T, LuaValue>::value, T>::type get() {
			std::optional<LuaValue> value;
			this->putOnTop([&](lua_State *state) {
				value = LuaValue::peek(state);
			});
			return value.value_or(LuaValue());
		}

		template <typename T>
		typename std::enable_if<std::is_convertible<LuaValue, T>::value && !std::is_same<T, LuaValue>::value, T>::type get() {
			return this->get<LuaValue>().get<T>();
		}

		template <typename T>
		typename std::enable_if<std::is_base_of<LuaData, T>::value>::type set(T value) {
			this->setValue([&value](lua_State *state) {
				value.push(state);
			});
		}

		void set(LuaData &value) {
			this->setValue([&value](lua_State *state) {
				value.push(state);
			});
		}

		template <typename T>
		typename std::enable_if<!std::is_base_of<LuaData, T>::value>::type set(T value) {
			LuaValue val = LuaValue::create<T>(value);
			this->setValue([&val](lua_State *state) {
				val.push(state);
			});
		}
	};

	class LuaGlobalVariable : public LuaReference {
	 public:
		LuaGlobalVariable(lua_State *state, const std::string &name) : state(state), name(name) {}
	
		void putOnTop(std::function<void (lua_State *)>) override;
		void setValue(std::function<void (lua_State *)>) override;
	 private:
		lua_State *state;
		std::string name;
	};

	class LuaStackReference : public LuaReference {
	 public:
		LuaStackReference(lua_State *state, int index) : state(state), index(index) {}

		void putOnTop(std::function<void (lua_State *)>) override;
		void setValue(std::function<void (lua_State *)> gen) override;
	 private:
		lua_State *state;
		int index;
	};

	class LuaReferenceHandle {
	 public:
	 	LuaReferenceHandle() : state(nullptr), ref(nullptr) {}
		LuaReferenceHandle(lua_State *state, std::unique_ptr<LuaReference> ref = nullptr) : state(state), ref(std::move(ref)) {}
		LuaReferenceHandle(const LuaReferenceHandle &);

		LuaReference &getReference() const;
		bool exists();
		LuaType getType();
		LuaReferenceHandle operator[](const std::string &);
		LuaReferenceHandle operator[](lua_Integer);
		LuaReferenceHandle &operator=(LuaData &);
		LuaReferenceHandle &operator=(const LuaReferenceHandle &);

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
	 	lua_State *state;
		std::unique_ptr<LuaReference> ref;
	};

	class LuaRegistryReference : public LuaReference {
	 public:
		LuaRegistryReference(lua_State *, int = -1);
		virtual ~LuaRegistryReference();

		void putOnTop(std::function<void (lua_State *)>) override;
		void setValue(std::function<void (lua_State *)>) override;
	 private:
		lua_State *state;
		int ref;
	};

	class LuaTableField : public LuaReference {
	 public:
		LuaTableField(LuaReferenceHandle ref, const std::string &name) : ref(ref), name(name) {}
		
		void putOnTop(std::function<void (lua_State *)>) override;
		void setValue(std::function<void (lua_State *)>) override;
	 private:
		LuaReferenceHandle ref;
		std::string name;
	};

	class LuaArrayField : public LuaReference {
	 public:
		LuaArrayField(LuaReferenceHandle ref, lua_Integer index) : ref(ref), index(index) {}

		void putOnTop(std::function<void (lua_State *)>) override;
		void setValue(std::function<void (lua_State *)>) override;
	 private:
		LuaReferenceHandle ref;
		lua_Integer index;
	};
}

#endif
