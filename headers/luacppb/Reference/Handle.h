#ifndef LUACPPB_REFERENCE_HANDLE_H_
#define LUACPPB_REFERENCE_HANDLE_H_

#include "luacppb/Meta.h"
#include "luacppb/Reference/Base.h"
#include "luacppb/Value/Native.h"
#include "luacppb/Invoke/Native.h"
#include "luacppb/Invoke/Invoke.h"
#include <memory>
#include <utility>
#include <type_traits>

namespace LuaCppB {

	class LuaReferenceHandle {
	 public:
	 	LuaReferenceHandle();
		LuaReferenceHandle(lua_State *, std::unique_ptr<Internal::LuaReference>);
		LuaReferenceHandle(const LuaReferenceHandle &);
		LuaReferenceHandle(LuaReferenceHandle &&);

		Internal::LuaReference &getReference() const;
		LuaCppRuntime &getRuntime() const;
		bool exists();
		LuaType getType();
		LuaReferenceHandle operator[](const std::string &);
		LuaReferenceHandle operator[](const char *);
		LuaReferenceHandle operator[](lua_Integer);
		LuaReferenceHandle &operator=(const LuaReferenceHandle &);
		LuaValue operator*();

		LuaReferenceHandle getMetatable();
		void setMetatable(LuaData &);
		void setMetatable(LuaData &&);

		template <typename T>
		typename std::enable_if<!std::is_same<T, LuaReferenceHandle>::value, LuaReferenceHandle>::type &operator=(T &);

		template <typename T>
		typename std::enable_if<!std::is_same<T, LuaReferenceHandle>::value, LuaReferenceHandle>::type &operator=(T &&);

		template <typename T, typename Type = typename std::enable_if<!std::is_class<T>::value || Internal::LuaReferenceGetSpecialCase<T>::value, T>::type>
		operator T ();

		template <typename T, typename Type = typename std::enable_if<std::is_class<T>::value && !Internal::LuaReferenceGetSpecialCase<T>::value, T>::type>
		operator T& ();

		template <typename T>
		typename std::enable_if<!Internal::is_instantiation<std::function, T>::value, T>::type get();

		template <typename ... A>
		Internal::LuaFunctionCallResult operator()(A &&...) const;
	 private:
	 	lua_State *state;
		std::unique_ptr<Internal::LuaReference> ref;
	};
}

#include "luacppb/Reference/Impl/Handle.h"

#endif