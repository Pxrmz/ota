#ifndef STRING_PRINTF_H
#define STRING_PRINTF_H

#include <stdarg.h>

void StringAppendF(char **dst, const char *format, ...);
char *StringPrintf(const char *fmt, ...);
char *StringVPrintf(const char *fmt, va_list ap);

#define string_append_free StringAppendF
#define string_aprintf     StringPrintf
#define string_vaprintf    StringVPrintf

#endif
