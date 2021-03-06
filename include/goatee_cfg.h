#ifndef _GOATEE_CFG_INC
#define _GOATEE_CFG_INC
#include "libstring.h"
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include "goatee_logger.h"
#include "goatee_hashmap.h"

void goatee_setup_basic_table(lua_State *L);
string goatee_dump_file(const char *filename);
char *goatee_trim_spaces(const char *in);
lua_State *goatee_getcfg(lua_State *L, const string in);
goatee_hashmap *goatee_parse_file(char *in);
#endif
