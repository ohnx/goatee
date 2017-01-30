#include "goatee_run.h"

void setup_basic_table(lua_State *L) {
    lua_newtable(L);
    lua_getglobal(L, "ipairs");
    lua_setfield(L, -2, "ipairs");
    lua_getglobal(L, "table");
    lua_setfield(L, -2, "table");
}

string goatee_run(lua_State *L, const char *in) {
    /* the lua_state has just the env that we want to run the template under
     * 
     * stack looks as follows:
     *
     * -1 env table
     */
    int stackSize, shouldRedo = 0;
    string out;
    
    if (L == NULL) {
        shouldRedo = 1;
        L = luaL_newstate();
        luaL_openlibs(L);
    }
    
    stackSize = lua_gettop(L);
    if (stackSize == 0) setup_basic_table(L);

    switch(lua_type(L, stackSize)) {
        case LUA_TTABLE:
            /* nothing needed to do */
            break;
        default:
            /* just push an empty table to stack */
            setup_basic_table(L);
            break;
    }
    
    /* load the template string */
    if (luaL_loadstring(L, in) != 0) {
        return NULL;
    }
    
    /* rearrange the stack so table is on top
     * right now, the stack looks like
     * 
     * -1 loaded string
     * -2 env table
     *
     * we need it to be like
     * 
     * -1 env table
     * -2 loaded string
     *
     * for the next block of code, so we do a little shuffle...
     */
    lua_pushvalue(L, -2);
    lua_pushvalue(L, -2);
    lua_remove(L, -3);
    lua_remove(L, -3);
    
    /* set the upvalue (aka the running environment for the code) */
    lua_setupvalue(L, -2, 1);
    
    /*
     * for this pcall to work, we need a function at top of stack
     * and the env to be set
     */
    if (lua_pcall(L, 0, 1, 0) != 0) {
        return NULL;
    }
    
    out = string_dup((const string)lua_tostring(L, -1));
    
    /*
     * Clean up lua if needed
     */
    if (shouldRedo) lua_close(L);
    
    return out;
}
