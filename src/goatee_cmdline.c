#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "goatee_gen.h"
#include "goatee_run.h"
#include "goatee_logger.h"
#include "goatee_cfg.h"
#include "goatee_hashmap.h"

#define __VERSION "v1.1.0 (happy llama)"

extern char **environ;

/*
 * you expect() something to happen.
 * if it does not, error with the string given
 */
#define expect(x, y) if(!(x)) error(y);

static goatee_logger *gl_cmd = NULL;

extern string goatee_gen_handle_comment(string all, string in);
extern string goatee_gen_handle_exec(string all, string in);
extern string goatee_gen_handle_var(string all, string in);
extern string goatee_gen_handle_normal(string all, string in);
extern string goatee_gen_noHeader(const string in, struct handlerInfo *modifiers, goatee_logger *glin);
string goatee_gen_handle_include_custom(string all, string in);

string dir_prefix;
static struct handlerInfo customModifiers[] = {
    {'#', '#', &goatee_gen_handle_comment},
    {'%', '%', &goatee_gen_handle_exec},
    {'{', '}', &goatee_gen_handle_var},
    {'+', '+', &goatee_gen_handle_include_custom},
    {0,0, NULL}
};

string goatee_gen_handle_include_custom(string all, string in) {
    string fileIn, tmp, tmp3;
    char *tmp2;
    
    if (!dir_prefix) {
        /* Disabled including files */
        gl_cmd->log(gl_cmd, GLL_ERR, "File including has been disabled, but template file tried to include a file!");
        return all;
    }
    
    tmp2 = goatee_trim_spaces(in);
    if (*tmp2 == '.') {
         /* Template tried to jump out of jail */
        gl_cmd->log(gl_cmd, GLL_WARN, "Template file tried to include a file in parent directory!");
        return all;
    }
    
    tmp = string_append(string_dup(dir_prefix), string_temporary(string_mknew(tmp2)));
    free(tmp2);
    fileIn = goatee_dump_file(tmp);
    
    tmp3 = string_append(string_mknew("Attemping to load "), string_temporary(string_mknew(tmp)));
    
    if (gl_cmd->level <= GLL_INFO) {
        gl_cmd->log(gl_cmd, GLL_INFO, tmp3);
    }
    
    if (fileIn == NULL) {
        if (gl_cmd->level > GLL_INFO) {
            gl_cmd->log(gl_cmd, GLL_WARN, tmp3);
        }

        gl_cmd->log(gl_cmd, GLL_WARN, "File not found.");
    
        string_free(tmp);
        string_free(tmp3);
        return all;
    }
    
    string_free(tmp);
    string_free(tmp3);
    
    tmp = goatee_gen_noHeader(fileIn, customModifiers, gl_cmd);
    string_free(fileIn);
    
    return string_append(all, string_temporary(
        string_appendv(3, "\n", string_temporary(tmp), "\n")
    ));
}

static void error(char *str) {
    goatee_logger_log(gl_cmd, GLL_FATAL, str);
}

void print_usage(char *argv[]) {
    printf("goatee "__VERSION" help:\n");
    printf("goatee is a templating engine that is super-extensible.\n" \
           "This is a sample command-line tool that provides basic rendering capabilities.\n" \
           "For more information about goatee, see https://masonx.ca/goatee\n");
    printf("-----\nUsage:\n");
    printf("-h\n\tShow this help message\n");
    printf("-i <filename>\n\tSpecify the input file\n\tIf none, stdin will be used.\n");
    printf("-v\n\tBe verbose (Print logs to stdout even if no error)\n");
    printf("-e\n\tRead environment variables into global table\n");
    printf("-l\n\tPrint only the output lua code from goatee_gen; do not run it.\n");
    printf("-p\n\tPrefix includes with something to force trap within a directory.\n"\
           "\tSet to NULL to disable including other files.\n");
    printf("-u\n\tUnsafe mode; allow all standard lua functions to be run from without a template.\n"\
           "\tTemplates normally run under a sandboxed environment; for help, please see https://masonx.ca/goatee\n");
    printf("-f <filename>\n\tRead contents of file into a global table.\n" \
           "\tContents of the file should follow the format; for help, please see https://masonx.ca/goatee\n");
    printf("<filename>\n\tOutput filename.\n\tIf none, stdout will be used.\n");
    printf("-----\nExample invocation:\n%s -ei file.in file.out\n", argv[0]);
}

int goatee_hashmap_iterator_printf(void *context, const char *key, void *value) {
    lua_State *L = (lua_State *)context;
    char *keyFirst, *keySecond, *dotLocation;
    
    /* expecting luaRef to be on top of stack */
    
    dotLocation = strchr(key, '.');
    /* handle cases where we need a table */
    if (dotLocation != NULL) {
        keyFirst = strdup(key);
        keySecond = keyFirst + (dotLocation-key);
        
        /* set the dot to null */
        *(keySecond++) = '\0';
        
        /* get env table value */
        lua_getfield(L, -1, keyFirst);

        if (!lua_istable(L, -1)) {
            /* need to create the table 
            printf("creating table `%s` to insert key `%s`\n", keyFirst, key); */
            lua_newtable(L);

            /* insert the value 
            printf("inserting into table `%s` key `%s` value `%.5s`\n", keyFirst, keySecond, (char *)value);*/
            lua_pushstring(L, keySecond);
            lua_pushstring(L, value);
            
            lua_settable(L, -3);

            lua_setfield(L, -3, keyFirst);

            lua_pop(L, 1);
        } else {
            /* insert the value 
            printf("inserting into table %s key %s value %.5s\n", keyFirst, keySecond, (char *)value); */
            lua_pushstring(L, keySecond);
            lua_pushstring(L, value);

            lua_settable(L, -3);
            lua_pop(L, 1);
        }
        
        free(keyFirst);
    } else {
        /* get env table value */
        lua_getfield(L, -1, key);

         if (!lua_istable(L, -1)) {
            lua_pushstring(L, (char *)value);
            /*printf("all ok here!");stackDump(L);fflush(stdout);*/
            lua_setfield(L, -3, key);
            
            lua_pop(L, 1);
         } else {
             printf("tried to set %s but it is a table\n", key);

             lua_pop(L, 1);
         }
    }
    
    return 0;
}

int main(int argc, char *argv[]) {
    char *in = NULL, *out = NULL, *outFinal = NULL, *fin = NULL, *fparse = NULL, *outFile = NULL;
    int readenv = 0, verbosity = GLL_ERR, onlygen = 0, unsafe = 0, useCustM = 0;
    lua_State *L = NULL;
    FILE *of;
    char c;
    
    while ((c = getopt(argc, argv, "hlup:vef:i:")) != -1) {
        switch (c) {
        case 'h':
            print_usage(argv);
            return 0;
        case 'l':
            onlygen = 1;
            break;
        case 'v':
            verbosity = GLL_INFO;
            break;
        case 'u':
            unsafe = 1;
            break;
        case 'e':
            readenv = 1;
            break;
        case 'f':
            fin = optarg;
            break;
        case 'i':
            fparse = optarg;
            break;
        case 'p':
            useCustM = 1;
            if (!strcmp(optarg, "NULL")) dir_prefix = NULL;
            else dir_prefix = string_mknew(optarg);
            break;
        case '?':
            fprintf(stderr, "Unknown option character `%c`.\n", optopt);
            print_usage(argv);
            return 1;
        default:
            fprintf(stderr, "???\n");
        }
    }
    
    gl_cmd = goatee_logger_new(verbosity);
    
    if (unsafe) {
        L = luaL_newstate();
        luaL_openlibs(L);
        /* push global env to stack */
        lua_getglobal(L, "_G");
    }
    
    if (readenv) {
        int i = 0;
        goatee_hashmap *envs = goatee_hashmap_new();
        char *poseq;
        
        if (!L) {
            L = luaL_newstate();
            luaL_openlibs(L);
            goatee_setup_basic_table(L);
        }
        
        while (environ[i]) {
            poseq = strchr(environ[i], '=');
            
            *poseq = '\0';
            goatee_hashmap_put(envs, environ[i], poseq+1);
            *poseq = '=';
            
            i++;
        }
        
        goatee_hashmap_iterate(envs, goatee_hashmap_iterator_printf, L);
        goatee_hashmap_destroy(envs);
    }
    
    if (fin) {
        char *tmp;
        goatee_hashmap *vars;

        if (!L) {
            L = luaL_newstate();
            luaL_openlibs(L);
            goatee_setup_basic_table(L);
        }

        tmp = goatee_dump_file(fin);
        expect(tmp != NULL, "Could not read file!");

        vars = goatee_parse_file(tmp);        
        expect(vars != NULL, "Failed reading file!");
        
        goatee_hashmap_iterate(vars, goatee_hashmap_iterator_printf, L);
        goatee_hashmap_destroy(vars);
        string_free(tmp);
    }
    
    if (!fparse) {
        /* read from stdin until EOF reached (^D) */
        char *tmp = calloc(256, sizeof(char)), buffer[256];
        
        /* read */
        while (fgets(buffer, 256 , stdin)) {
            /* realloc for tmp, adding 1 for null */
            tmp = realloc(tmp, strlen(tmp)+1+strlen(buffer));
            if (!tmp) { /* failed memory allocation */
                fprintf(stderr, "Out of Memory Error.\n");
                fflush(stdout);
                return -1337;
            }
            strcat(tmp, buffer); /* note a '\n' is appended here everytime */
        }
        
        /* place in string_mknew */
        in = string_mknew(tmp);
        free(tmp);
    } else {
        /* read in file */
        in = goatee_dump_file(fparse);
        expect(in != NULL, "Could not read file!");
    }
    
    /* set output file (default NULL) */
    if (optind <= argc)
        outFile = argv[optind];

    /* generate the template string that will get run through lua */
    if (!useCustM)
        out = goatee_gen(in, NULL, gl_cmd);
    else
        out = goatee_gen(in, customModifiers, gl_cmd);
    expect(out != NULL, "Failed to compile template into lua code!");

    /* that's all that's done so far... print out output */
    gl_cmd->log(gl_cmd, GLL_INFO, "goatee_gen finished");
    
    if (onlygen) {
        outFinal = out;
        out = NULL;
        goto end;
    }

    /* generate the output */
    outFinal = goatee_run(L, out, gl_cmd);
    expect(outFinal != NULL, "Failed to run compiled template!");
    
    gl_cmd->log(gl_cmd, GLL_INFO, "goatee_run finished");
    
end:
    if (outFile) {
        of = fopen(outFile, "w");
        fprintf(of, "%s", outFinal);
        fclose(of);
    } else {
        printf("%s", outFinal);
    }
    
    /* be responsible... free memory */
    string_free(in);
    if (out) string_free(out);
    string_free(outFinal);
    if (L) lua_close(L);
    
    fprintf(stderr, "%s", gl_cmd->messages);
    goatee_logger_destroy(gl_cmd);
    
    return 0;
}
