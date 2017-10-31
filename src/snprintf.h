/*
 * snprintf.h -- interface for our fixup vsnprintf() etc. functions
 */

#ifndef SNPRINTF_H
#define SNPRINTF_H 1

#include "epoc.h"
#include <stdarg.h> /* ... */

int fixup_vsnprintf(char *str, size_t count, const char *fmt, va_list args);
int fixup_snprintf(char *str, size_t count, const char *fmt, ...);

#endif /* snprintf.h */
