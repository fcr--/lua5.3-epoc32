#include <e32std.h>
#include <oplapi.h>

extern "C" {
  extern void epoc_init(void);

#include <signal.h>
#include <stdlib.h>
#include "lua.h"

  /* Correct EPOC bug where if there are no characters are converted,
   *endptr is set to NULL rather than s */
  double epoc_strtod(const char *s, char **endptr) {
    double res = strtod(s, endptr);
    if (*endptr == NULL) *endptr = (char *)s;
    return res;
  }

  void lsys_unloadlib (void *lib) {
    ((RLibrary*)lib)->Close();
  }

  void * lsys_load (lua_State *L, const char *path, int seeglb) {
    RLibrary * library = new RLibrary;
    (void)(seeglb); // ignored
    TInt res = library->Load(_L(path));
    if (res != KErrNone) {
      lua_pushfstring(L, "OPL error %d\n", OplAPI::MapError(res));
      return NULL;
    }
    // don't forget to add -uid2 0x4c756121 when creating the DLL!
    if (library->Type()[1].iUid != KLuaUid) {
      lua_pushfstring(L, "Library has incorrect UID2 = 0x%08x, "
          "0x%08x expected\n", library->Type()[1].iUid, KLuaUid);
      library->Close();
      delete library;
      return NULL;
    }
    return (void*)library;
  }

  lua_CFunction lsys_sym (lua_State *L, void *lib, const char *sym) {
    (void)(sym); // sym is ignored since function at ordinal 1 is used.
    // Make sure your luaopen_foobar function is exported in that index!
    return (lua_CFunction)((RLibrary *)lib)->Lookup(1);
  }
}

static void handler(TExcType) {}

void epoc_init(void) { RThread().SetExceptionHandler(handler,KExceptionFpe); }
