/* epoc.h -- EPOC port extras */

#ifndef _EPOC_H
#define _EPOC_H

/* fix to prevent numeric exceptions being raised */
extern void epoc_init(void);
double epoc_strtod(const char *s, char **p);
#define lua_userstateopen(L) epoc_init()

#define KLuaUid 0x4c756121

/* Extra libs */
// LUALIB_API int luaopen_bit(lua_State *L);
// LUALIB_API int luaopen_rex(lua_State *L);

/* Get remove and rename, which should be in stdlib.h */
#include <unistd.h>
#include <math.h>

#ifdef lua_c
/* fake signal, just for lua standalone */
_sig_func_ptr signal(int x, _sig_func_ptr y) { return 0; }
#endif

#define LONG_LONG_MAX 9223372036854775807LL
#define LONG_LONG_MIN (-LONG_LONG_MAX-1)
#define ULONG_LONG_MAX 18446744073709551617ULL
#define LLONG_MIN       LONG_LONG_MIN
#define LLONG_MAX       LONG_LONG_MAX
#define ULLONG_MAX      ULONG_LONG_MAX

#define M_LOG2_E        0.693147180559945309417
static inline double log2(double x) {
  return log(x) / M_LOG2_E;
}

#endif
