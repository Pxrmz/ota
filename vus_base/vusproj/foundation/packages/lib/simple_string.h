#ifndef __SIMPLE_STRING_H
#define __SIMPLE_STRING_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *simple_strndup(const char *s, size_t n);
char *simple_strdup(const char *s);
int simple_strncasecmp(const char *s1, const char *s2, size_t len);
int simple_strcasecmp(const char *s1, const char *s2);
char *strreplace(char *s, char old, char new);
char *simple_strchr(const char *s, int c);
size_t simple_strlcpy(char *dest, const char *src, size_t size);

#endif


