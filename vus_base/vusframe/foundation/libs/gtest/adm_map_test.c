/***************************************************************************
 *
 * Copyright (c) 2016 zkdnfcf, Inc. All Rights Reserved
 * $Id$
 *
 **************************************************************************/

#include "adm_map.h"
#include <stdio.h>
#include <unistd.h>

int main()
{
    root_t* tree = map_new();
    char* key = "hello";
    char* word = "world";
    map_put(tree, key, word);

    char* key1 = "hello 1";
    char* word1 = "world 1";
    map_put(tree, key1, word1);

    char* key2 = "hello 1";
    char* word2 = "world 2 change";
    map_put(tree, key2, word2);

    char* data1 = map_get_val(tree, "hello 1");

    if (data1 != NULL)
        printf("%s\n", data1);

    map_t* node;
    for (node = map_first(tree); node; node = map_next(node)) {
        printf("%s\n", map_get_key(node));
        printf("%s\n", map_get_value(node));
    }

    // free map if you don't need
    map_free(tree);
    return 0;
}

/* vim: set ts=4 sw=4 sts=4 tw=100 */
