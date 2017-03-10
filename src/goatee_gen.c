#include "goatee_gen.h"

static string goatee_gen_internal(const string in, struct handlerInfo *modifiers, int useHeaders);

static goatee_logger *gl_gen = NULL;
struct handlerInfo *modifiers_gen;

/**
 * These handler functions need to return all
 * in case the memory address changed
 */
string goatee_gen_handle_comment(string all, string in) {
    return all;
}

string goatee_gen_handle_exec(string all, string in) {
    /* sing string_appendv here since it creates a new string, allowing us to free it back in main function */
    return string_append(all, string_temporary(string_appendv(2, in, "\n")));
}

string goatee_gen_handle_var(string all, string in) {
    return string_append(all, string_temporary(
        string_appendv(3, "_ret[#_ret+1] = ", in, "\n")
    ));
}

string goatee_gen_handle_include(string all, string in) {
    string fileIn, tmp, tmp3;
    char *tmp2;
    
    tmp2 = goatee_trim_spaces(in);
    fileIn = goatee_dump_file(tmp2);
    
    tmp3 = string_append("Attemping to load ", tmp2);
    
    if (gl_gen->level <= GLL_INFO) {
        gl_gen->log(gl_gen, GLL_INFO, tmp3);
    }
    
    if (fileIn == NULL) {
        if (gl_gen->level > GLL_INFO) {
            gl_gen->log(gl_gen, GLL_WARN, tmp3);
        }

        gl_gen->log(gl_gen, GLL_WARN, "File not found.");
    
        free(tmp2);
        string_free(tmp3);
        return all;
    }
    
    free(tmp2);
    string_free(tmp3);
    
    tmp = goatee_gen_internal(fileIn, modifiers_gen, 0);
    string_free(fileIn);
    
    return string_append(all, string_temporary(
        string_appendv(3, "\n", string_temporary(tmp), "\n")
    ));
}

static string goatee_gen_handle_normal(string all, string in) {
    return string_append(all, string_temporary(
        string_appendv(3, "_ret[#_ret+1] = [[\n", in, "]]\n")
    ));
}

struct handlerInfo defaultModifiers[] = {
    {'#', '#', &goatee_gen_handle_comment},
    {'%', '%', &goatee_gen_handle_exec},
    {'{', '}', &goatee_gen_handle_var},
    {'+', '+', &goatee_gen_handle_include},
    {0,0, NULL}
};

string goatee_gen(const string in, struct handlerInfo *modifiers, goatee_logger *glin) {
    gl_gen = glin;
    return goatee_gen_internal(in, modifiers, 1);
}

static string goatee_gen_internal(const string in, struct handlerInfo *modifiers, int useHeaders) {
    string out, tmp;
    char starttag;
    char *c, *bP;
    char end[3] = {0,'}',0}; /* null terminator */
    unsigned int pos, inL, b, i;
    
    /* check if input is NULL */
    if (in == NULL) return in;
    
    /* check if modifiers is nothing, and use default ones if yes */
    if (!modifiers) {
        modifiers = defaultModifiers;
    }
    /* in case goatee_gen_handle_include is used */
    modifiers_gen = modifiers;

    /* get length of string */
    inL = string_length(in);

    /* initial values */
    out = useHeaders ? string_mknew("_ret = {}\n") : string_new();
    pos = 0;
    
    while (pos < inL) {
        /* Look for the start of a block */
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
                out = goatee_gen_handle_normal(out, tmp);
                string_free(tmp);
            }
            out = goatee_gen_handle_normal(out, "{");
            pos = b+1;
        } else {
            /* add all text up until this block */
            if (b > pos) {
                tmp = string_new();
                tmp = string_copy(tmp, &in[pos], 0, b-pos);
                out = goatee_gen_handle_normal(out, tmp);
                string_free(tmp);
            }
            
            /* get the start tag */
            starttag = in[b+1];
            
            /* look up end tag and handler */
            for (i = 0; modifiers[i].start != 0; i++)
                if (modifiers[i].start == starttag) break;
            
            /* end tag is now in modifiers[i] */
            
            if (starttag == '\0') { /* special case: end of string */
                gl_gen->log(gl_gen, GLL_ERR, "Unexpected end of string! (expecting close)");
                goto failure;
            } else if (modifiers[i].start == 0) { /* could not exist... */
                /* unrecognized starting */
                tmp = string_new();
                tmp = string_copy(tmp, &in[b-1], 0, 3);
                out = goatee_gen_handle_normal(out, tmp);
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
    
    /* Add any text after the last block */
    tmp = string_new();
    tmp = string_copy(tmp, &in[pos], 0, inL-pos);
    out = goatee_gen_handle_normal(out, tmp);
    string_free(tmp);
    
    if (useHeaders) out = string_append(out, "\nreturn table.concat(_ret)\n");
    
    return out;
failure:
    string_free(out);
    return NULL;
}
