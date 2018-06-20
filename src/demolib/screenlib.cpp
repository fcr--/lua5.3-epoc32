#include <e32std.h>
#include <coemain.h>

extern "C" {
#include "lua.h"
#include "lauxlib.h"
}

struct State {
  CWsScreenDevice * screenDevice;
  RBackedUpWindow * background;
  CWindowGc * gc;
};

static inline State *getstate(lua_State * L) {
  return (State*)lua_touserdata(L, lua_upvalueindex(1));
}

static int setpencolor(lua_State * L) {
  TInt r = luaL_checkinteger(L, 1);
  TInt g = luaL_checkinteger(L, 2);
  TInt b = luaL_checkinteger(L, 3);
  getstate(L)->gc->SetPenColor(TRgb(r, g, b));
  return 0;
}

static int setpensize(lua_State * L) {
  TInt w = luaL_checkinteger(L, 1);
  TInt h = luaL_checkinteger(L, 2);
  getstate(L)->gc->SetPenSize(TSize(w, h));
  return 0;
}

static int moveto(lua_State * L) {
  TInt x = luaL_checkinteger(L, 1);
  TInt y = luaL_checkinteger(L, 2);
  getstate(L)->gc->MoveTo(TPoint(x, y));
  return 0;
}

static int moveby(lua_State * L) {
  TInt dx = luaL_checkinteger(L, 1);
  TInt dy = luaL_checkinteger(L, 2);
  getstate(L)->gc->MoveBy(TPoint(dx, dy));
  return 0;
}

static int plot(lua_State * L) {
  TInt x = luaL_checkinteger(L, 1);
  TInt y = luaL_checkinteger(L, 2);
  getstate(L)->gc->Plot(TPoint(x, y));
  return 0;
}

static int drawlineto(lua_State * L) {
  TInt x = luaL_checkinteger(L, 1);
  TInt y = luaL_checkinteger(L, 2);
  getstate(L)->gc->DrawLineTo(TPoint(x, y));
  return 0;
}

static int drawlineby(lua_State * L) {
  TInt dx = luaL_checkinteger(L, 1);
  TInt dy = luaL_checkinteger(L, 2);
  getstate(L)->gc->DrawLineBy(TPoint(dx, dy));
  return 0;
}

static TRect parse_rect(lua_State * L, int index) {
  TRect t;
  luaL_checktype(L, index, LUA_TTABLE);
  if (lua_geti(L, index, 1) && lua_geti(L, index, 2)) {
    t.iTl = TPoint(luaL_checkinteger(L, -2), luaL_checkinteger(L, -1));
  } else {
    luaL_error(L, "missing upper left point in TRect");
  }
  if (lua_geti(L, index, 3) && lua_geti(L, index, 4)) {
    t.iBr = TPoint(luaL_checkinteger(L, -2), luaL_checkinteger(L, -1));
  } else if (lua_getfield(L, index, "w") && lua_getfield(L, index, "w")) {
    t.iBr = t.iTl + TPoint(luaL_checkinteger(L, -2), luaL_checkinteger(L, -1));
  } else {
    luaL_error(L, "missing bottom right point (or 'w', 'h') in TRect");
  }
  t.Normalize();
  return t;
}

static int scroll(lua_State * L) {
  struct State * state = getstate(L);
  luaL_checktype(L, 1, LUA_TTABLE);
  if (lua_geti(L, 1, 1) != LUA_TNUMBER ||
      lua_geti(L, 1, 2) != LUA_TNUMBER) {
    return luaL_error(L, "expected offset");
  }
  TPoint offset = TPoint(luaL_checkinteger(L, -2), luaL_checkinteger(L, -1));
  bool has_clip = false, has_rect = false;
  TRect clip, rect;
  if (lua_getfield(L, 1, "clip")) {
    clip = parse_rect(L, lua_gettop(L));
    has_clip = true;
  }
  if (lua_getfield(L, 1, "rect")) {
    rect = parse_rect(L, lua_gettop(L));
    has_rect = true;
  }
  if (has_clip && has_rect) {
    state->background->Scroll(clip, offset, rect);
  } else if (has_rect) {
    state->background->Scroll(offset, rect);
  } else if (has_clip) {
    state->background->Scroll(clip, offset);
  } else {
    state->background->Scroll(offset);
  }
  return 0;
}

static int drawrect(lua_State * L) {
  struct State * state = getstate(L);
  state->gc->DrawRect(parse_rect(L, 1));
  return 0;
}

static int deactivate(lua_State * L) {
  struct State * state = getstate(L);
  state->gc->Deactivate();
  state->background = NULL;
  return 0;
}

static int activate(lua_State * L) {
  State * state = getstate(L);
  if (state->background) {
    state->gc->Deactivate();
    state->background = NULL;
  }
  if (lua_islightuserdata(L, 1)) {
    state->background = (RBackedUpWindow*)lua_touserdata(L, 1);
  } else {
    luaL_checkinteger(L, 1); // idl
    lua_getglobal(L, "opl");
    if (lua_getfield(L, -1, "_windowfromidl") != LUA_TFUNCTION)
      return luaL_error(L, "opl._windowfromidl must be defined or call this "
	  "function with a valid RBackedUpWindow* lightuserdata instead");
    lua_pushvalue(L, 1);
    lua_call(L, 1, 1); // opl._windowfromidl(idl)
    state->background = (RBackedUpWindow*)lua_touserdata(L, -1);
  }
  state->gc->Activate(*state->background);
  return 0;
}

EXPORT_C int luaopen_screenlib(lua_State * L) {
  if (lua_getglobal(L, "opl") != LUA_TTABLE)
    return luaL_error(L, "opl must be a table");
  if (lua_getfield(L, -1, "_screendevice") != LUA_TLIGHTUSERDATA)
    return luaL_error(L, "opl._screendevice must be correctly defined");

  struct State *state = new State;
  state->screenDevice = (CWsScreenDevice*)lua_touserdata(L, -1);
  state->screenDevice->CreateContext(state->gc);
  state->background = NULL;

  lua_newtable(L);
  lua_pushlightuserdata(L, state);
  luaL_setfuncs(L, (struct luaL_Reg[]) {
    {"setpencolor", &setpencolor},
    {"setpensize", &setpensize},
    {"moveto", &moveto},
    {"moveby", &moveby},
    {"plot", &plot},
    {"drawlineto", &drawlineto},
    {"drawlineby", &drawlineby},
    {"scroll", &scroll},
    {"drawrect", &drawrect},
    {"deactivate", &deactivate},
    {"activate", &activate},
    {NULL, NULL}
  }, 1);
  // try to activate using IdL #1, where IDs correspond to the OPL function gUSE
  lua_getfield(L, -1, "activate");
  lua_pushinteger(L, 1);
  if (lua_pcall(L, 1, 0, 0)) lua_pop(L, 1);
  return 1;
}

GLDEF_C TInt E32Dll(TDllReason /*aReason*/) { // no need for thread local storage
  return(KErrNone);
}
