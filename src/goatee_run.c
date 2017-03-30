#include "goatee_run.h"

string goatee_run(lua_State *L, const char *in, goatee_logger *gl) {
    /* the lua_state has just the env that we want to run the template under
     * 
     * stack looks as follows:
     *
     * -1 env table
     */
    int stackSize, shouldRedo = 0;
    string out = NULL;
    
    if (L == NULL) {
        shouldRedo = 1;
        L = luaL_newstate();
        luaL_openlibs(L);
        gl->log(gl, GLL_INFO, "Creating new lua instance");
    }
    
    stackSize = lua_gettop(L);
    if (stackSize == 0) {
        goatee_setup_basic_table(L);
        goto skipTable;
    }

    switch(lua_type(L, stackSize)) {
        case LUA_TTABLE:
            /* duplicate table since setupvalue kills it */
            lua_pushvalue(L, -1);
            break;
        default:
            /* just push an empty table to stack */
            goatee_setup_basic_table(L);
            break;
    }
    
    skipTable:
    
    /* load the template string */
    if (luaL_loadstring(L, in) != 0) {
        out = string_mknew(lua_tostring(L, -1));
        gl->log(gl, GLL_ERR, out);
        out = NULL;
        goto end;
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
    lua_pushvalue(L, -1);
    lua_pushvalue(L, -3);
    lua_remove(L, -3);
    lua_remove(L, -3);
    
    /* set the upvalue (aka the running environment for the code) */
    lua_setupvalue(L, -2, 1);
    
    /*
     * for this pcall to work, we need a function at top of stack
     * and the env to be set
     */
    if (lua_pcall(L, 0, 1, 0) != 0) {
        out = string_mknew(lua_tostring(L, -1));
        gl->log(gl, GLL_ERR, out);
        out = NULL;
        goto end;
    }
    
    out = string_dup((const string)lua_tostring(L, -1));

    /* pop the string */
    lua_pop(L, 1);

end:
    /*
     * Clean up lua if needed
     */
    if (shouldRedo) lua_close(L);
    
    return out;
}
