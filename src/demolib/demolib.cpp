#include <e32std.h>

extern "C" {
#include "lua.h"
#include "lauxlib.h"
}

static int hello(lua_State * L) {
  lua_pushstring(L, "hello!");
  return 1;
}

EXPORT_C int luaopen_demolib(lua_State * L) {
  lua_newtable(L);
  luaL_setfuncs(L, (struct luaL_Reg[]){
    {"hello", &hello},
    {NULL, NULL}
  }, 0);
  return 1;
}

GLDEF_C TInt E32Dll(TDllReason /*aReason*/) { // no need for thread local storage
  return(KErrNone);
}
