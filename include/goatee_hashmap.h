#include <string.h>
#include <stdlib.h>

#ifndef __GOATEE_HASHMAP_INC
#define __GOATEE_HASHMAP_INC

typedef struct _goatee_hashmap_entry {
    char *key;
    void *value;
} goatee_hashmap_entry;

typedef struct _goatee_hashmap_entry_list {
    int vlen;
    int vroom;
    goatee_hashmap_entry *values;
} goatee_hashmap_entry_list;

typedef struct _goatee_hashmap {
    goatee_hashmap_entry_list *buckets;
} goatee_hashmap;

typedef int (*goatee_hashmap_iterator)(void *context, const char *key, void *value);

goatee_hashmap *goatee_hashmap_new();
void goatee_hashmap_put(goatee_hashmap *in, const char *key, void *value);
void *goatee_hashmap_get(goatee_hashmap *in, const char *key);
int goatee_hashmap_iterate(goatee_hashmap *in, goatee_hashmap_iterator iter, void *context);
void goatee_hashmap_empty(goatee_hashmap *in);
void goatee_hashmap_destroy(goatee_hashmap *in);
#endif
