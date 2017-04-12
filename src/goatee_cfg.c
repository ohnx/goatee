#include "goatee_cfg.h"

string goatee_dump_file(const char *filename) {
    char *buffer = 0;
    string tmp;
    long length;
    FILE *f;
    
    f = fopen (filename, "rb");
    
    if (f) {
        fseek(f, 0, SEEK_END);
        length = ftell(f);
        fseek(f, 0, SEEK_SET);
        buffer = calloc(length+1, 1);
        if (buffer) {
            fread (buffer, 1, length, f);
            buffer[length] = '\0';
        }
        fclose (f);
    }
    
    if (!buffer) return NULL;
    tmp = string_mknew(buffer);
    if (buffer) free(buffer);
    
    return tmp;
}

char *goatee_trim_spaces(const char *in) {
    const char *startLocation;
    int length = 0;
    
    /* loop until the character is no longer a space */
    while (*in == ' ') in++;
    
    /* save the current location */
    startLocation = in;
    
    /* Loop until the character is a space or null (ie, string ended) */
    while (*in != ' ' && *in != '\0') {
        length++;
        in++;
    }
    
    /* return the newly allocated string */
    return strndup(startLocation, length);
}

void goatee_setup_basic_table(lua_State *L) {
    lua_newtable(L);
    lua_getglobal(L, "ipairs");
    lua_setfield(L, -2, "ipairs");
    lua_getglobal(L, "table");
    lua_setfield(L, -2, "table");
    lua_getglobal(L, "pairs");
    lua_setfield(L, -2, "pairs");
    lua_getglobal(L, "tonumber");
    lua_setfield(L, -2, "tonumber");
}

goatee_hashmap *goatee_parse_file(char *in) {
    goatee_hashmap *vars = goatee_hashmap_new();
    char *poseq, *posnl = in, *cpos = in;

    while (1) {
        /* find next newline */
        posnl = strchr(posnl, '\n');
        
        if (posnl == NULL) { /* none found, look for end of string */
            posnl = strchr(cpos, '\0'); /* jump to end of string */

            /* find equals sign, if none, return */
            poseq = strchr(cpos, '=');
            if (poseq == NULL) return NULL;
            *poseq = '\0';

            /* add to goatee_hashmap */
            goatee_hashmap_put(vars, cpos, poseq+1);
            break;
        }
        
        /* newline may have been escaped  - NYI
        if (*(posnl-1) == '\\') {posnl++;continue;}*/
        *posnl = '\0';
        
        /* find equals sign, if none, return */
        poseq = strchr(cpos, '=');
        if (poseq == NULL) return NULL;
        *poseq = '\0';
        
        /* add to goatee_hashmap */
        goatee_hashmap_put(vars, cpos, poseq+1);
        cpos = ++posnl;
    }
    
    /* return goatee_hashmap */
    return vars;
}
