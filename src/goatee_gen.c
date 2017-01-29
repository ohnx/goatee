#include "goatee_gen.h"

/**
 * These handler functions need to return all
 * in case the memory address changed
 */
string handle_comment(string all, string in) {
    return all;
}

string handle_exec(string all, string in) {
    all = string_append(all, in);
    all = string_append(all, "\n");
    return all;
}

string handle_var(string all, string in) {
    all = string_append(all, "_ret[#_ret+1] = ");
    all = string_append(all, in);
    all = string_append(all, "\n");
    return all;
}

string handle_normal(string all, string in) {
    all = string_append(all, "_ret[#_ret+1] = [[\n");
    all = string_append(all, in);
    all = string_append(all, "]]\n");
    return all;
}

typedef string(*goatee_handler)(string all, string in);

struct {
        char start;
        char end;
        goatee_handler handler;
} modifiers[] = {
    {'#', '#', &handle_comment},
    {'%', '%', &handle_exec},
    {'{', '}', &handle_var},
    {0,0, NULL}
};

string goatee_gen(const string in) {
    string out, tmp;
    char starttag;
    char *c, *bP;
    char end[3] = {0,'}',0}; /* null terminator */
    unsigned int pos, inL, b, i;
    
    /* check if input is NULL */
    if (in == NULL) return in;

    /* get length of string */
    inL = string_length(in);

    /* initial values */
    out = string_dup("_ret = {}\n");
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
    
    /* Add any text after the last block */
    tmp = string_new();
    tmp = string_copy(tmp, &in[pos], 0, inL-pos);
    out = handle_normal(out, tmp);
    string_free(tmp);
    
    out = string_append(out, "\nreturn table.concat(_ret)\n");
    
    return out;
failure:
    string_free(out);
    return NULL;
}
