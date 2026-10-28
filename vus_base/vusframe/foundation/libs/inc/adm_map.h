/***************************************************************************
 *
 * Copyright (c) 2016 zkdnfcf, Inc. All Rights Reserved
 * $Id$
 *
 **************************************************************************/

#ifndef _MAP_H
#define _MAP_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct map map_t;
typedef struct rb_root root_t;

/*new and delete*/
root_t* map_new();
void map_free(root_t* root);

/*get and set*/
void* map_get_val(const root_t* root, const char* key, unsigned int* valSize);
bool map_put(root_t* root, const char* key, void* val, unsigned int valSize);
void map_erase(root_t* root, const char* key);

/*iteration*/
map_t* map_first(const root_t* tree);
map_t* map_next(map_t* map_node);

/*get key and value from map_t node*/
char* map_get_key(const map_t* node);
void* map_get_value(const map_t* node);

#ifdef __cplusplus
}
#endif
#endif  //_MAP_H
