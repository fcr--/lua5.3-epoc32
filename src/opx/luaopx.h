// luaopx.h

#include <e32base.h>
#include <oplapi.h>
#include <opx.h>
extern "C" {
#include "lua.h"
}

// UID & version number; must match those in DECLARE OPX of .oxh
const TInt KUidOpxLua=0x10008F9A;
const TInt KOpxLuaVersion=0x100;

// An instance of COpxLua is created on loading the OPX
// NewOpxL() stores the object pointer in the TLS
class COpxLua : public COpxBase
{
public:
  static COpxLua* NewLC(OplAPI& api);
  virtual void RunL(TInt proc);
  virtual TInt CheckVersion(TInt v);
  int LuaDispatch (lua_State *L) const;
private:
  // OPL procedures
  enum TExtensions { ELua=1, ELuaOpen, ELuaClose, ELuaRegister,
		     ELuaGetGlobal, ELuaSetGlobal, ELuaGetGlobalPtr, ELuaSetGlobalPtr };
  void Lua() const;
  void LuaOpen() const;
  void LuaClose() const;
  void LuaRegister() const;
  void LuaGetGlobal() const;
  void LuaSetGlobal() const;
  void LuaGetGlobalPtr() const;
  void LuaSetGlobalPtr() const;
  // Other private methods
  void Error(lua_State *L, TInt err) const;
  const char *CheckString(lua_State *L, TAny *sp, int nargs) const;
  double CheckNumber(lua_State *L, TAny *sp, int nargs) const;
  COpxLua(OplAPI& api);
  ~COpxLua();
};
