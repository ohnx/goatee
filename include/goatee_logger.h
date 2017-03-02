#ifndef _GOATEE_LOGGER
#define _GOATEE_LOGGER
#include "libstring.h"

typedef enum goatee_logger_level {
    GLL_INFO = 0,
    GLL_WARN = 1,
    GLL_ERR = 2,
    GLL_FATAL = 3
} goatee_logger_level;

typedef struct goatee_logger {
    string messages;
    goatee_logger_level level;
    void (*log)(struct goatee_logger *gl, goatee_logger_level level, const string msg);
} goatee_logger;

goatee_logger *goatee_logger_new(goatee_logger_level min_level);
void goatee_logger_destroy(goatee_logger *gl);
void goatee_logger_log(goatee_logger *gl, goatee_logger_level level, const string msg);

#endif
