#ifndef LUACPPB_LUAREFERENCE_H_
#define LUACPPB_LUAREFERENCE_H_

#include "luacppb/Base.h"
#include "luacppb/LuaState.h"
#include "luacppb/LuaType.h"
#include "luacppb/Function.h"
#include <type_traits>
#include <string>
#include <functional>
#include <memory>

namespace LuaCppB {

	class LuaReferenceHandle;

	class LuaReference {
	 public:
		virtual ~LuaReference() = default;

		template <typename T>
		typename std::enable_if<std::is_integral<T>::value, T>::type get() {
			lua_State *state = this->get_value();
			if (state) {
				lua_Integer value = lua_tointeger(state, -1);
				lua_pop(state, -1);
				return static_cast<T>(value);
			} else {
				return 0;
			}
		}

		template <typename T>
		typename std::enable_if<std::is_integral<T>::value, bool>::type set(T value) {
			return this->set_value([value](lua_State *state) {
				lua_pushinteger(state, static_cast<lua_Integer>(value));
			});
		}
		
		template <typename T>
		typename std::enable_if<std::is_same<T, LuaCFunction>::value, bool>::type set(LuaCFunction value) {
			return this->set_value([value](lua_State *state) {
				lua_pushcfunction(state, value);
			});
		}

		template <typename T, typename R, typename ... A>
		typename std::enable_if<std::is_same<T, CFunctionCall<R, A...>>::value, bool>::type set(CFunctionCall<R, A...> value) {
			return this->set_value([&value](lua_State *state) {
				value.makeClosure(state);
			});
		}

		template <typename T, typename C, typename R, typename ... A>
		typename std::enable_if<std::is_same<T, CMethodCall<C, R, A...>>::value, bool>::type set(CMethodCall<C, R, A...> value) {
			return this->set_value([&value](lua_State *state) {
				value.makeClosure(state);
			});
		}
	 
		virtual lua_State *get_value() = 0;
		virtual bool set_value(std::function<void (lua_State *)>) = 0;
	};

	class LuaGlobalVariable : public LuaReference {
	 public:
		LuaGlobalVariable(LuaState &, const std::string &);
	
		lua_State *get_value() override;
		bool set_value(std::function<void (lua_State *)>) override;
	 private:
		LuaState &state;
		std::string name;
	};

	class LuaIndexReference : public LuaReference {
	 public:
		LuaIndexReference(LuaState &, lua_Integer);

		lua_State *get_value() override;
		bool set_value(std::function<void (lua_State *)>) override;
	 private:
		LuaState state;
		lua_Integer index;
	};

	class LuaReferenceHandle {
	 public:
		LuaReferenceHandle(std::shared_ptr<LuaReference>);

		LuaReference &getReference();
		LuaReferenceHandle operator[](const std::string &);
		LuaReferenceHandle operator[](lua_Integer);
		bool exists();
		LuaType getType();
		
		LuaReferenceHandle &operator=(LuaCFunction fn) {
			this->ref->set<LuaCFunction>(fn);
			return *this;
		}

		template <typename R, typename ... A>
		LuaReferenceHandle &operator=(R (*fn)(A...)) {
			this->ref->set<CFunctionCall<R, A...>>(CFunctionCall<R, A...>(fn));
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
	 private:
		std::shared_ptr<LuaReference> ref;
	};

	class LuaTableField : public LuaReference {
	 public:
		LuaTableField(LuaReferenceHandle, const std::string &);
		
		lua_State *get_value() override;
		bool set_value(std::function<void (lua_State *)>) override;
	 private:
		LuaReferenceHandle ref;
		std::string name;
	};

	class LuaArrayField : public LuaReference {
	 public:
		LuaArrayField(LuaReferenceHandle, lua_Integer);
		
		lua_State *get_value() override;
		bool set_value(std::function<void (lua_State *)>) override;
	 private:
		LuaReferenceHandle ref;
		lua_Integer index;
	};
}

#endif
