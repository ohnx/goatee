#ifndef _GOATEE_LOGGER
#define _GOATEE_LOGGER
#include "libstring.h"

typedef enum goatee_logger_level {
    GLL_INFO,
    GLL_WARN,
    GLL_ERR
} goatee_logger_level;

typedef struct goatee_logger {
    string *messages;
    goatee_logger_level level;
} goatee_logger;

goatee_logger *goatee_logger_new(goatee_logger_level level);
void goatee_logger_log(goatee_logger *gl, goatee_logger_level level, string msg);

#endif
