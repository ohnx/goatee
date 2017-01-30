#ifndef _GOATEE_CFG
#define _GOATEE_CFG
#include "libstring.h"
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

string goatee_run(lua_State *L, const char *in);
#endif
