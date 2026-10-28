#ifndef __SBUF_H
#define __SBUF_H

#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

struct sbuf {
    char *data;
    size_t limit;
    size_t size;
    int flags;
};

struct sbuf *sbuf_alloc(size_t size);
struct sbuf *sbuf_initial_alloc(const char *data, size_t size);
struct sbuf *sbuf_initial_alloc_extra(const char *data, size_t size, size_t extra_size);
struct sbuf *sbuf_refer_alloc(const char *data, size_t size);
struct sbuf *sbuf_dup(struct sbuf *sbuf);
char *sbuf_strdup(struct sbuf *sbuf);
struct sbuf *sbuf_incremental_dup(struct sbuf *sbuf, size_t incremetal_size);
struct sbuf *sbuf_realloc(struct sbuf *sbuf, size_t size);
struct sbuf *sbuf_incremental_realloc(struct sbuf *sbuf, size_t incremetal_size);
size_t sbuf_sizeof(struct sbuf *sbuf);
size_t sbuf_get_free(struct sbuf *sbuf);

int sbuf_add_data(struct sbuf *sbuf, const char *data, size_t size);
int sbuf_dump_data(struct sbuf *sbuf, char *data, size_t size);
void sbuf_free(struct sbuf *sbuf);

int sbuf_set_flags(struct sbuf *sbuf, int flag);
int sbuf_get_flags(struct sbuf *sbuf);

#ifdef __cplusplus
}
#endif

#endif
