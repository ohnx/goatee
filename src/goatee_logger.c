#include "goatee_logger.h"

#ifdef __linux__
    #define KNRM  "\x1B[0m"
    #define KRED  "\x1B[31m"
    #define KGRN  "\x1B[32m"
    #define KYEL  "\x1B[33m"
    #define KBLU  "\x1B[34m"
    #define KMAG  "\x1B[35m"
    #define KCYN  "\x1B[36m"
    #define KWHT  "\x1B[37m"
    #define RESET "\033[0m"
#else
    #define KNRM  ""
    #define KRED  ""
    #define KGRN  ""
    #define KYEL  ""
    #define KBLU  ""
    #define KMAG  ""
    #define KCYN  ""
    #define KWHT  ""
    #define RESET ""
#endif

goatee_logger *goatee_logger_new(goatee_logger_level min_level) {
    goatee_logger *n = calloc(1, sizeof(goatee_logger));
    n->level = min_level;
    n->messages = string_new();
    n->log = &goatee_logger_log;
    return n;
}

char *logStrings[] = {RESET"["KBLU"INFO"RESET"] ",
                      RESET"["KYEL"WARN"RESET"] ",
                      RESET"["KRED"ERRR"RESET"] ",
                      RESET"["KMAG"FATL"RESET"] "};

void goatee_logger_destroy(goatee_logger *gl) {
    string_free(gl->messages);
    free(gl);
}

void goatee_logger_log(goatee_logger *gl, goatee_logger_level level, const string msg) {
    if (gl != NULL && level >= gl->level)
        gl->messages = string_append(gl->messages, string_temporary(
            string_appendv(3, logStrings[level], msg, "\n")    
        ));
    
    if (level == GLL_FATAL) {
        if (gl != NULL) {
            fprintf(stderr, "%s", gl->messages);
            goatee_logger_destroy(gl);
        } else fprintf(stderr, "%s%s\n", logStrings[level], msg);
        exit(-1337);
    }
}
