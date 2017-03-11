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


/* WARN: GARBAGE CODE */
#if 0
lua_State *goatee_getcfg(lua_State *L, const string in) {
    unsigned int pos, inL, b;
    char *bP;
    
    /* we will populate the lua_State with an env table */
    if (L == NULL) {
        L = luaL_newstate();
        luaL_openlibs(L);
    }
    
    goatee_setup_basic_table(L);
    
    inL = string_length(in);
    
    /* loop through input string */
    while (pos < inL) {
        bP = strchr(&in[pos], '{');
        
        /* check for ending */
        if (!bP) break;
        
        /* not done yet! */
        b = bP - in;

        /* check if this is a block or escaped { */ 
        if (b != 0 && in[b-1] == '\\') {
            /* generate string so far */
            if (b > pos+1) {
                tmp = string_new();
                tmp = string_copy(tmp, &in[pos], 0, b-pos-1);
                out = handle_normal(out, tmp);
                string_free(tmp);
            }
            out = handle_normal(out, "{");
            pos = b+1;
        } else {
            /* add all text up until this block */
            if (b > pos) {
                tmp = string_new();
                tmp = string_copy(tmp, &in[pos], 0, b-pos);
                out = handle_normal(out, tmp);
                string_free(tmp);
            }
            
            /* get the start tag */
            starttag = in[b+1];
            
            /* look up end tag and handler */
            for (i = 0; modifiers[i].start != 0; i++)
                if (modifiers[i].start == starttag) break;
            
            /* end tag is now in modifiers[i] */
            
            if (starttag == '\0') { /* special case: end of string */
                
                goto failure;
            } else if (modifiers[i].start == 0) { /* could not exist... */
                /* unrecognized starting */
                tmp = string_new();
                tmp = string_copy(tmp, &in[b-1], 0, 3);
                out = handle_normal(out, tmp);
                string_free(tmp);
                pos = b+2;
            } else { /* or exist */
                /* find the ending now - skip past current */
                end[0] = modifiers[i].end;
                c = strstr(&in[b+2], end);
                
                if (c == NULL) {/* could not find! */
                    
                    goto failure;
                }
                
                tmp = string_new();
                tmp = string_copy(tmp, &in[b+2], 0, (c-in)-2-b);
                out = modifiers[i].handler(out, tmp);
                string_free(tmp);
                
                /* done! */
                pos = c-in+2;
            }
        }
    }
    
    
    return L;
}
#endif