#include "luacppb/LuaReference.h"

namespace LuaCppB {
	
	LuaGlobalVariable::LuaGlobalVariable(LuaState &state, const std::string &name)
		: state(state), name(name) {}

	lua_State *LuaGlobalVariable::get_value() {
		lua_State *state = this->state.getState();
		if (state) {
			lua_getglobal(state, this->name.c_str());
			if (lua_isnone(state, -1)) {
				lua_pop(state, -1);
				state = nullptr;
			}
		}
		return state;
	}

	bool LuaGlobalVariable::set_value(std::function<void (lua_State *)> gen) {
		lua_State *state = this->state.getState();
		if (state) {
			gen(state);
			lua_setglobal(state, this->name.c_str());
		}
		return state != nullptr;
	};

	LuaIndexReference::LuaIndexReference(LuaState &state, lua_Integer index)
		: state(state), index(index) {}

	lua_State *LuaIndexReference::get_value() {
		lua_State *state = this->state.getState();
		lua_copy(state, this->index, -1);
		return state;
	}

	bool LuaIndexReference::set_value(std::function<void (lua_State *)> gen) {
		return false;
	}

	LuaTableField::LuaTableField(LuaReferenceHandle ref, const std::string &name)
		: ref(ref), name(name) {}

	lua_State *LuaTableField::get_value() {
		lua_State *state = this->ref.getReference().get_value();
		if (state) {
			if (lua_istable(state, -1)) {
				lua_getfield(state, -1, this->name.c_str());
				if (lua_isnone(state, -1)) {
					lua_pop(state, -1);
					state = nullptr;
				}
			} else {
				lua_pop(state, -1);
				state = nullptr;
			}
		}
		return state;
	}

	bool LuaTableField::set_value(std::function<void (lua_State *)> gen) {
		lua_State *state = this->ref.getReference().get_value();
		if (state) {
			if (lua_istable(state, -1)) {
				gen(state);
				lua_setfield(state, -2, this->name.c_str());
				lua_pop(state, -1);
			} else {
				lua_pop(state, -1);
				state = nullptr;
			}
		}
		return state != nullptr;
	}

	LuaArrayField::LuaArrayField(LuaReferenceHandle ref, lua_Integer index)
		: ref(ref), index(index) {}

	lua_State *LuaArrayField::get_value() {
		lua_State *state = this->ref.getReference().get_value();
		if (state) {
			if (lua_istable(state, -1)) {
				lua_geti(state, -1, this->index);
				if (lua_isnone(state, -1)) {
					lua_pop(state, -1);
					state = nullptr;
				}
			} else {
				lua_pop(state, -1);
				state = nullptr;
			}
		}
		return state;
	}

	bool LuaArrayField::set_value(std::function<void (lua_State *)> gen) {
		lua_State *state = this->ref.getReference().get_value();
		if (state) {
			if (lua_istable(state, -1)) {
				gen(state);
				lua_seti(state, -2, this->index);
				lua_pop(state, -1);
			} else {
				lua_pop(state, -1);
				state = nullptr;
			}
		}
		return state != nullptr;
	}

	LuaReferenceHandle::LuaReferenceHandle(std::shared_ptr<LuaReference> ref)
		: ref(ref) {}

	LuaReference &LuaReferenceHandle::getReference() {
		return *this->ref;
	}

	LuaReferenceHandle LuaReferenceHandle::operator[](const std::string &name) {
		return LuaReferenceHandle(std::make_shared<LuaTableField>(*this, name));
	}

	LuaReferenceHandle LuaReferenceHandle::operator[](lua_Integer index) {
		return LuaReferenceHandle(std::make_shared<LuaArrayField>(*this, index));
	}

	bool LuaReferenceHandle::exists() {
		lua_State *state = this->ref->get_value();
		bool exists = false;
		if (state) {
			exists = !(lua_isnone(state, -1) || lua_isnil(state, -1));
			lua_pop(state, -1);
		}
		return exists;
	}

	LuaType LuaReferenceHandle::getType() {
		lua_State *state = this->ref->get_value();
		if (state) {
			LuaType type = static_cast<LuaType>(lua_type(state, -1));
			lua_pop(state, -1);
			return type;
		} else {
			return LuaType::Nil;
		}
	}
}
