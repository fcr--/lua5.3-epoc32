// luaopx.cpp
// Lua OPX (Lua <-> OPL interworking)
// Reuben Thomas
// Lua 4.0 version 16/1/01-30/1/03
// Lua 5.0 version 31/5-12/10/03
// Lua 5.1 version 25/3/07

#include "luaopx.h"
#include <e32std.h>
#include <oplerr.h>
extern "C" {
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}


// Utility functions and methods

// Convert a C++ string to a zero-delimited C string
static char *cstring(TPtrC8 s) {
  TInt len = s.Length();
  char *buf = (char *)malloc(len + 1);
  strncpy(buf, (char *)s.Ptr(), len);
  buf[len] = '\0';
  return buf;
}

// Raise a type error in Lua
static void type_error (lua_State *L, int narg, int t) {
  char buff[50];
  sprintf(buff, "%.8s expected, got %.8s", lua_typename(L, t),
                                           lua_typename(L, lua_type(L, narg)));
  luaL_argerror(L, narg, buff);
}

// Raise an OPL error in Lua
void COpxLua::Error(lua_State *L, TInt err) const {
  luaL_error(L, "OPL error %d", iOplAPI.MapError(err));
}

// Get a number from Lua stack, and exit on error
double COpxLua::CheckNumber (lua_State *L, TAny *sp, int narg) const {
  double d = lua_tonumber(L, narg);
  if (d == 0 && !lua_isnumber(L, narg)) {  /* avoid extra test when d is not 0 */
    iOplAPI.SetStackPtr(sp);
    type_error(L, narg, LUA_TNUMBER);
  }
  return d;
}

// Get a string from Lua stack, and exit on error
const char *COpxLua::CheckString (lua_State *L, TAny *sp, int narg) const {
  const char *s = lua_tostring(L, narg);
  if (!s) {
    iOplAPI.SetStackPtr(sp);
    type_error(L, narg, LUA_TSTRING);
  }
  return s;
}

// Perform leaving operation and exit if it was left
#define TRAPERR(stat) TRAP(err, (stat)); if (err) { iOplAPI.SetStackPtr(sp); this->Error(L, err); }

// Character of an OPL identifier
#define isidchar(c) (isalnum(c) || (c) == '_')
// 1st character of an OPL identifier
#define isidfirst(c) (isalpha(c) || (c) == '_')

// Parse an OPL identifier
static char *id(char *s) {
  if (!isidfirst(*s)) User::Leave(KOplErrInvalidArgs);
  do { s++; } while (isidchar(*s));
  return s;
}

// Convert an OPL return type character into a code
static TReturnType type(char **s) {
  switch (**s) {
    case '%': (*s)++; return EReturnInt;    break;
    case '&': (*s)++; return EReturnLong;   break;
    case '$': (*s)++; return EReturnString; break;
    default:  return EReturnFloat;
  }
}

// Check for a character in an OPL function prototype, and trim trailing spaces
static void check(char **s, char c) {
  if (**s != c) User::Leave(KOplErrInvalidArgs);
  while (*++(*s) == ' ');
}

// Dispatch a callback to OPL (called by dispatch, below)
int COpxLua::LuaDispatch(lua_State *L) const {
  const char *func = lua_tostring(L, lua_upvalueindex(1));
  TPtrC8 ptr = TPtrC8((TText8 *)func);
  TInt argn = (TInt)lua_tonumber(L, lua_upvalueindex(2));
  TInt act_args = lua_gettop(L);
  if (act_args != argn)
    luaL_error(L, "wrong number of arguments (%d expected, %d received)", argn, act_args);
  TAny *sp = iOplAPI.StackPtr();
  TInt err;
  TRAPERR(iOplAPI.InitCallbackL(ptr));
  for (TInt i = 1; i <= argn; i++) {
    switch ((int)lua_tonumber(L, lua_upvalueindex(i + 3))) {
      case EReturnInt: {
        TInt16 val = (TInt16)this->CheckNumber(L, sp, i);
        TRAPERR(iOplAPI.PushParamL(val));
        break;
      }
      case EReturnLong: {
        TInt32 val = (TInt32)this->CheckNumber(L, sp, i);
        TRAPERR(iOplAPI.PushParamL(val));
        break;
      }
      case EReturnFloat: {
        TReal64 val = (TReal64)this->CheckNumber(L, sp, i);
        TRAPERR(iOplAPI.PushParamL(val));
        break;
      }
      case EReturnString: {
        TPtrC8 val = TPtrC8((TText8 *)this->CheckString(L, sp, i));
        TRAPERR(iOplAPI.PushParamL(val));
        break;
      }
    }
  }
  TReturnType ret = (TReturnType)(int)lua_tonumber(L, lua_upvalueindex(3));
  if ((err = iOplAPI.CallProcedure(ret)))
      this->Error(L, err);
  switch (ret) {
    case EReturnInt:    lua_pushinteger(L, iOplAPI.PopInt16());  break;
    case EReturnLong:   lua_pushinteger(L, iOplAPI.PopInt32());  break;
    case EReturnFloat:  lua_pushnumber(L, (double)iOplAPI.PopReal64()); break;
    case EReturnString: {
      TPtrC8 str = iOplAPI.PopString8();
      lua_pushlstring(L, (char *)str.Ptr(), str.Length());
      break;
    }
  }
  return 1;
}

// Dispatch a callback to OPL (Lua needs to call a C rather than a C++ procedure)
static int dispatch(lua_State *L) {
  return ((COpxLua *)Dll::Tls())->LuaDispatch(L);
}

// OPL extension procedures

void COpxLua::Lua() const {
  TPtrC8 str = iOplAPI.PopString8();
  lua_State *L = (lua_State *)iOplAPI.PopInt32();
  int top = lua_gettop(L);
  iOplAPI.Push((TInt32) (luaL_loadbuffer(L, (char *)str.Ptr(), str.Length(), "OPL") || lua_pcall(L, 0, LUA_MULTRET, 0)));
  lua_settop(L, top);
}

static int opl_windowfromidl(lua_State *L) {
  int id = lua_tointeger(L, 1);
  OplAPI * api = (OplAPI*)lua_touserdata(L, lua_upvalueindex(1));
  lua_pushlightuserdata(L, &api->WindowFromIdL(id));
  return 1;
}

void COpxLua::LuaOpen() const {
  lua_State *L = luaL_newstate();
  luaL_openlibs(L);
  lua_getglobal(L, "opl");
  if (lua_isnil(L, -1)) {
    lua_pop(L, 1);
    lua_newtable(L);
  }
  // opl._screendevice : CWsScreenDevice*
  lua_pushlightuserdata(L, iOplAPI.ScreenDevice());
  lua_setfield(L, -2, "_screendevice");

  // opl._background : RBackedUpWindow*
  lua_pushlightuserdata(L, &iOplAPI);
  lua_pushcclosure(L, &opl_windowfromidl, 1);
  lua_setfield(L, -2, "_windowfromidl");
  lua_setglobal(L, "opl");
  // non-core libs
  //luaopen_bit(L);
  //luaopen_rex(L);
  iOplAPI.Push((TInt32)L);
}

void COpxLua::LuaClose() const {
  lua_State *L = (lua_State *)iOplAPI.PopInt32();
  lua_close(L);
  iOplAPI.Push(0.0); // return value
}

void COpxLua::LuaRegister() const {
  char *name = cstring(iOplAPI.PopString8());
  char *proto = cstring(iOplAPI.PopString8());
  lua_State *L = (lua_State *)iOplAPI.PopInt32();
  char *args = id(proto);
  TInt funcLen = args - proto;
  luaL_checkstack(L, 3, "Lua stack exhausted");
  lua_pushlstring(L, proto, funcLen); // function name = upvalue 1
  lua_pushnumber(L, 0.0);       // number of args (not known yet) = upvalue 2
  TReturnType ret = type(&args);
  lua_pushnumber(L, (double)ret); // return type = upvalue 3
  check(&args, ':');
  TInt argn = 0;
  if (*args) {
    check(&args, '(');
    do {
      args = id(args);
      luaL_checkstack(L, 1, "too many arguments");
      lua_pushnumber(L, (double)type(&args));
      argn++;
    } while (*args++ == ',');
    check(&--args, ')');
  }
  if (argn) {
    lua_pushnumber(L, (double)argn);
    lua_replace(L, -argn - 3);  // overwrite upvalue 2
  }
  free(proto);
  // 1 upvalue for PROC name, 1 for return type, 1 for no. of args & 1 for each arg type
  lua_pushcclosure(L, dispatch, argn + 3);
  lua_setglobal(L, name);
  free(name);
  iOplAPI.Push(0.0); // return value
}

void COpxLua::LuaGetGlobal () const {
  char *str = cstring(iOplAPI.PopString8());
  lua_State *L = (lua_State *)iOplAPI.PopInt32();
  lua_getglobal(L, str);
  free(str);
  TAny *sp = iOplAPI.StackPtr();
  TPtrC8 val = TPtrC8((TText8 *)this->CheckString(L, sp, -1));
  TInt err;
  TRAPERR(iOplAPI.PushL(val));
  lua_pop(L, 1);
}

void COpxLua::LuaSetGlobal () const {
  TPtrC8 val = iOplAPI.PopString8();
  char *id = cstring(iOplAPI.PopString8());
  lua_State *L = (lua_State *)iOplAPI.PopInt32();
  lua_pushlstring(L, (char *)val.Ptr(), val.Length());
  lua_setglobal(L, id);
  free(id);
  iOplAPI.Push(0.0); // return value
}

void COpxLua::LuaGetGlobalPtr () const {
  TInt32 opladdr = iOplAPI.PopInt32();
  TText8 *buffer = iOplAPI.OffsetToAddrL(opladdr, 0);
  char *str = cstring(iOplAPI.PopString8());
  lua_State *L = (lua_State *)iOplAPI.PopInt32();
  lua_getglobal(L, str);
  free(str);
  TInt32 bytes = lua_rawlen(L, -1);
  TText8 *addr = (TText8 *)lua_tostring(L, -1);
  Mem::Copy(buffer, addr, bytes);
  iOplAPI.Push(0.0); // return value
  lua_pop(L, 1);
}

void COpxLua::LuaSetGlobalPtr () const {
  char *val = (char *)iOplAPI.OffsetToAddrL(iOplAPI.PopInt32(), 0);
  char *id = cstring(iOplAPI.PopString8());
  lua_State *L = (lua_State *)iOplAPI.PopInt32();
  lua_pushstring(L, val);
  lua_setglobal(L, id);
  free(id);
  iOplAPI.Push(0.0); // return value
}


// Other COpxLua methods

COpxLua::COpxLua(OplAPI& api) : COpxBase(api) {
  __DECLARE_NAME(_S("COpxLua"));
}

COpxLua::~COpxLua() { Dll::FreeTls(); } // free TLS when OPX is unloaded

COpxLua* COpxLua::NewLC(OplAPI& api) {
  COpxLua* opx = new(ELeave) COpxLua(api);
  CleanupStack::PushL(opx);
  return opx;
}


// COpxBase implementation

void COpxLua::RunL(TInt proc) {
  switch (proc) {
    case ELua:             Lua();             break;
    case ELuaOpen:         LuaOpen();         break;
    case ELuaClose:        LuaClose();        break;
    case ELuaRegister:     LuaRegister();     break;
    case ELuaGetGlobal:    LuaGetGlobal();    break;
    case ELuaSetGlobal:    LuaSetGlobal();    break;
    case ELuaGetGlobalPtr: LuaGetGlobalPtr(); break;
    case ELuaSetGlobalPtr: LuaSetGlobalPtr(); break;
    default:               User::Leave(KOplErrOpxProcNotFound);
  }
}

TBool COpxLua::CheckVersion(TInt v) {
  // OPX's major version must be greater than given major version
  if ((v & 0xff00) > (KOpxLuaVersion & 0xff00)) return EFalse;
  else return ETrue;
}


// DLL functions

// create COpxBase instance as required by OPL runtime, to be stored in the OPX's TLS
EXPORT_C COpxBase* NewOpxL(OplAPI& api) {
  COpxLua* tls = (COpxLua *)Dll::Tls();
  if (tls == NULL) { // Create COpxLua instance if OPX just loaded
    tls = COpxLua::NewLC(api);
    User::LeaveIfError(Dll::SetTls(tls));
    CleanupStack::Pop();
  }
  return (COpxBase *)tls;
}

EXPORT_C TUint Version() { return KOpxLuaVersion; }

GLDEF_C TInt E32Dll(TDllReason) { return(KErrNone); }
