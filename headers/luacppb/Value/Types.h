#ifndef LUACPPB_VALUE_TYPES_H_
#define LUACPPB_VALUE_TYPES_H_

#include "luacppb/Value/Base.h"
#include "luacppb/Reference/Registry.h"
#include "luacppb/Meta.h"
#include <string>

namespace LuaCppB {

	class LuaNil : public LuaValueBase {
	 public:
		void push(lua_State *) const override;
		static LuaNil get(lua_State *, int = -1);
	};

	class LuaInteger : public LuaValueBase {
	 public:
	 	LuaInteger();
		LuaInteger(lua_Integer);
		operator lua_Integer() const {
			return this->integer;
		}
		void push(lua_State *) const override;
		static LuaInteger get(lua_State *, int = -1);
	 private:
		lua_Integer integer;
	};

	class LuaNumber : public LuaValueBase {
	 public:
	 	LuaNumber();
	 	LuaNumber(lua_Number);
		operator lua_Number() const {
			return this->number;
		}
		void push(lua_State *) const override;
		static LuaNumber get(lua_State *, int = -1);
	 private:
		lua_Number number;
	};

	class LuaBoolean : public LuaValueBase {
	 public:
		LuaBoolean(bool);
		operator bool() const {
			return this->boolean;
		}
		void push(lua_State *) const override;
		static LuaBoolean get(lua_State *, int = -1);
	 private:
		bool boolean;
	};

	class LuaString : public LuaValueBase {
	 public:
		LuaString(const std::string &);
		LuaString(const char *);
	 	operator const std::string &() const {
			return this->string;
		}
		void push(lua_State *) const override;
		static LuaString get(lua_State *, int = -1);
	 private:
		std::string string;
	};

	typedef int (*LuaCFunction_ptr)(lua_State *);

	class LuaCFunction : public LuaValueBase {
	 public:
		LuaCFunction(LuaCFunction_ptr);
		operator LuaCFunction_ptr() const {
			return this->function;
		}
		void push(lua_State *) const override;
		static LuaCFunction get(lua_State *, int = -1);
	 private:
		LuaCFunction_ptr function;
	};

	class LuaReferenceHandle;

	class LuaReferencedValue : public LuaValueBase {
	 public:
	 	LuaReferencedValue();
		LuaReferencedValue(lua_State *, int = -1);
		LuaReferencedValue(const LuaReferencedValue &);
		
		void push(lua_State *) const override;

		template <typename T>
		operator T() {
			return this->convert<T>();
		}

	protected:
		template <typename T>
		T toPointer() const {
			T res = nullptr;
			handle.get([&](lua_State *state) {
				const void *ptr = lua_topointer(state, -1);;
				res = reinterpret_cast<T>(const_cast<void *>(ptr));
			});
			return res;
		}

	 private:
		template <typename T>
		typename std::enable_if<!std::is_same<T, LuaReferenceHandle>::value, T>::type convert() {
			static_assert(Internal::always_false<T>::value , "");
		}

		template <typename T>
		typename std::enable_if<std::is_same<T, LuaReferenceHandle>::value, T>::type convert();

	 	Internal::LuaSharedRegistryHandle handle;
	};

	class LuaTable : public LuaReferencedValue {
	 public:
	 	using LuaReferencedValue::LuaReferencedValue;
		static LuaTable get(lua_State *, int = -1);
		static LuaTable create(lua_State *);
	};

	class LuaUserData : public LuaReferencedValue {
	 public:
	 	using LuaReferencedValue::LuaReferencedValue;
		static LuaUserData get(lua_State *, int = -1);
		static LuaUserData create(lua_State *, std::size_t);

		template <typename T>
		T toPointer() const {
			return this->LuaReferencedValue::toPointer<T>();
		}
	};

	class LuaThread : public LuaReferencedValue {
	 public:
		using LuaReferencedValue::LuaReferencedValue;
		lua_State *toState() const;
		static LuaThread get(lua_State *, int = -1);
		static LuaThread create(lua_State *);
	};
}

#endif 