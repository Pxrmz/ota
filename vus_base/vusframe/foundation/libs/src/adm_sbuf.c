#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "adm_sbuf.h"
#include "assert_param.h"

int sbuf_set_flags(struct sbuf* sbuf, int flag)
{
    ASSERT_PARAM(sbuf != NULL);

    int old_flags = sbuf->flags;

    sbuf->flags |= flag;

    return old_flags;
}

int sbuf_get_flags(struct sbuf* sbuf)
{
    ASSERT_PARAM(sbuf != NULL);
    return sbuf->flags;
}

static struct sbuf* do_sbuf_initial_alloc_extra(const char* data, size_t size, size_t extra_size, int refer)
{
    struct sbuf* sbuf = NULL;

    if (refer) {
        /* Reference alloc */
        sbuf = malloc(sizeof(*sbuf) + extra_size);

        if (!sbuf) {
            return NULL;
        }

        sbuf->data = (char*)data;
        sbuf->limit = size;
        sbuf->size = size;
    } else {
        /* Non-refer alloc */
        sbuf = malloc(sizeof(*sbuf) + size + extra_size);

        if (!sbuf) {
            return NULL;
        }

        sbuf->data = (char*)(sbuf + 1);
        sbuf->limit = size + extra_size;

        if (!data) {
            sbuf->size = 0;
        } else {
            sbuf->size = size;
            memcpy(sbuf->data, data, size);
        }
    }

    return sbuf;
}

struct sbuf* sbuf_alloc(size_t size)
{
    ASSERT_PARAM(size > 0);

    return do_sbuf_initial_alloc_extra(NULL, size, 0, 0);
}

struct sbuf* sbuf_initial_alloc(const char* data, size_t size)
{
    ASSERT_PARAM(size > 0);
    ASSERT_PARAM(data != NULL);

    return do_sbuf_initial_alloc_extra(data, size, 0, 0);
}

struct sbuf* sbuf_initial_alloc_extra(const char* data, size_t size, size_t extra_size)
{
    ASSERT_PARAM(size > 0);
    ASSERT_PARAM(data != NULL);

    return do_sbuf_initial_alloc_extra(data, size, extra_size, 0);
}

struct sbuf* sbuf_refer_alloc(const char* data, size_t size)
{
    return do_sbuf_initial_alloc_extra(data, size, 0, 1);
}

static int do_add_data(struct sbuf* sbuf, const char* data, size_t size)
{
    char* sbuf_data;
    size_t copied_size;

    if (sbuf->size + size > sbuf->limit) {
        copied_size = sbuf->limit - sbuf->size;
    } else {
        copied_size = size;
    }

    sbuf_data = sbuf->data;
    memcpy(&sbuf_data[sbuf->size], data, copied_size);

    sbuf->size += copied_size;

    return copied_size;
}

int sbuf_add_data(struct sbuf* sbuf, const char* data, size_t size)
{
    ASSERT_PARAM(sbuf != NULL);
    ASSERT_PARAM(data != NULL);

    return do_add_data(sbuf, data, size);
}

int sbuf_dump_data(struct sbuf* sbuf, char* data, size_t size)
{
    size_t copied_size;

    ASSERT_PARAM(sbuf != NULL);
    ASSERT_PARAM(data != NULL);

    if (sbuf->size > size) {
        copied_size = size;
    } else {
        copied_size = sbuf->size;
    }

    memcpy(data, sbuf->data, copied_size);

    return copied_size;
}

size_t sbuf_sizeof(struct sbuf* sbuf)
{
    return sbuf->size;
}

size_t sbuf_get_free(struct sbuf* sbuf)
{
    return (sbuf->limit - sbuf->size);
}

/* From man realloc:
 *
 * The realloc() function changes the size of the memory block pointed to by ptr to size bytes.
 * The contents will be unchanged in the range from the start of the region up to the minimum
 * of the old and new sizes. If the new size is larger than the old size, the added memory will
 * not be initialized.  If ptr is NULL, then the call is equivalent to malloc(size), for all
 * values of size; if size is equal to zero, and ptr is not NULL, then the call is equivalent
 * to free(ptr). Unless ptr is NULL, it must have been returned by an earlier call to malloc(),
 * calloc() or realloc().  If the area pointed to was moved, a free(ptr) is done.
 *
 * NOTE1:
 * If realloc() fails, the original block is left untouched;
 * it is not freed or moved.
 */
static struct sbuf* do_realloc(struct sbuf* sbuf, size_t size)
{
    struct sbuf* new_sbuf = do_sbuf_initial_alloc_extra(NULL, size, 0, 0);

    if (new_sbuf) {
        do_add_data(new_sbuf, sbuf->data, sbuf->size);
        sbuf_free(sbuf);
    }

    return new_sbuf;
}

struct sbuf* sbuf_realloc(struct sbuf* sbuf, size_t size)
{
    ASSERT_PARAM(sbuf != NULL);
    ASSERT_PARAM(size > 0);

    return do_realloc(sbuf, size);
}

struct sbuf* sbuf_incremental_realloc(struct sbuf* sbuf, size_t incremetal_size)
{
    ASSERT_PARAM(sbuf != NULL);

    if (incremetal_size == 0) {
        return sbuf;
    }

    return do_realloc(sbuf, incremetal_size + sbuf->limit);
}

struct sbuf* sbuf_dup(struct sbuf* sbuf)
{
    struct sbuf* new_sbuf = do_sbuf_initial_alloc_extra(NULL, sbuf->limit, 0, 0);

    if (new_sbuf) {
        do_add_data(new_sbuf, sbuf->data, sbuf->size);
    }

    return new_sbuf;
}

char* sbuf_strdup(struct sbuf* sbuf)
{
    char* s = malloc(sbuf->size + 1);
    if (s) {
        memcpy(s, sbuf->data, sbuf->size);
        s[sbuf->size] = '\0';
    }

    return s;
}

struct sbuf* sbuf_incremental_dup(struct sbuf* sbuf, size_t incremetal_size)
{
    struct sbuf* new_sbuf = do_sbuf_initial_alloc_extra(sbuf->data, sbuf->size, sbuf->limit + incremetal_size, 1);

    if (new_sbuf) {
        do_add_data(new_sbuf, sbuf->data, sbuf->size);
    }

    return new_sbuf;
}

void sbuf_free(struct sbuf* sbuf)
{
    ASSERT_PARAM(sbuf != NULL);

    free(sbuf);
}
