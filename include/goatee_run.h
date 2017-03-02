#ifndef _GOATEE_CFG
#define _GOATEE_CFG
#include "libstring.h"
#include "goatee_cfg.h"
#include "goatee_logger.h"
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

string goatee_run(lua_State *L, const char *in, goatee_logger *gl);
#endif
