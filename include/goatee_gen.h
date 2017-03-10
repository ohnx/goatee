#ifndef _GOATEE_INC
#define _GOATEE_INC
#include "libstring.h"
#include "goatee_cfg.h"

typedef string(*goatee_gen_handler)(string all, string in);

struct handlerInfo {
    char start;
    char end;
    goatee_gen_handler handler;
};

string goatee_gen(const string in, struct handlerInfo *modifiers, goatee_logger *glin);
#endif
