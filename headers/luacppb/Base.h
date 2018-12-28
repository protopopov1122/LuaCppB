#ifndef LUACPPB_BASE_H_
#define LUACPPB_BASE_H_

#ifndef LUACPPB_C_MODE

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#define LUACPPB_COROUTINE_SUPPORT
#define LUACPPB_ERROR_SUPPORT
#define LUACPPB_EXCEPTION_PROCESSING

#else

extern "C" {
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
}

#endif

#endif
