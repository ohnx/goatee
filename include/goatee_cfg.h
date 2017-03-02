#ifndef _GOATEE_CFG_INC
#define _GOATEE_CFG_INC
#include "libstring.h"
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include "goatee_logger.h"

void goatee_setup_basic_table(lua_State *L);
string dumpFile(const char *filename);
char *strutil_trim_spaces(const char *in);
lua_State *goatee_getcfg(lua_State *L, const string in);
#endif
