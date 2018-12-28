#ifndef LUACPPB_BASE_H_
#define LUACPPB_BASE_H_

#include "luacppb/Config.h"

#ifdef LUACPPB_CXX_MODE

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#ifndef LUACPPB_NO_COROUTINE_SUPPORT
#define LUACPPB_COROUTINE_SUPPORT
#endif

#ifndef LUACPPB_NO_ERROR_SUPPORT
#define LUACPPB_ERROR_SUPPORT
#endif

#ifndef LUACPPB_NO_EXCEPTION_PROPAGATION
#define LUACPPB_EXCEPTION_PROPAGATION
#endif

#else

extern "C" {
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
}
#define LUACPPB_EXCEPTION_PROPAGATION

#endif

#endif
