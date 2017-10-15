BEGIN {
  print "#include \"lua.h\""
  print "#include \"lauxlib.h\""
  print "#include \"lualib.h\""
  print "#define LUA_API EXPORT_C"
  print "#define LUALIB_API EXPORT_C"
  print "#define LUAMOD_API EXPORT_C"

  IGNORED["lua_pushfstring"] = 1
  print "LUA_API const char *_lua_pushfstring (lua_State *L, const char *fmt, ...) {"
  print "  const char *ret;"
  print "  va_list argp;"
  print "  va_start(argp, fmt);"
  print "  ret = lua_pushvfstring(L, fmt, argp);"
  print "  va_end(argp);"
  print "  return ret;"
  print "}"

  IGNORED["luaL_error"] = 1 # just do a simple copy
  print "LUALIB_API int _luaL_error (lua_State *L, const char *fmt, ...) {"
  print "  va_list argp;"
  print "  va_start(argp, fmt);"
  print "  luaL_where(L, 1);"
  print "  lua_pushvfstring(L, fmt, argp);"
  print "  va_end(argp);"
  print "  lua_concat(L, 2);"
  print "  return lua_error(L);"
  print "}"

  CONDITIONALS["luaL_pushmodule"] = "defined(LUA_COMPAT_MODULE)"
  CONDITIONALS["luaL_openlib"] = "defined(LUA_COMPAT_MODULE)"
}

/LUA(LIB|MOD)?_API/ {
  name = $0; gsub(/ *\(.*/, "", name); gsub(/.*[ *]/, "", name)
  if (name in IGNORED || name in DEFINED) next;
  DEFINED[name] = 1
  if (name in CONDITIONALS) print "#if " CONDITIONALS[name]
  gsub(/[a-zA-Z_0-9]* *\(/, "_&")
  print
  args = $0
  while (/, *$/) {
    getline
    args = args $0
    print
  }
  gsub(/.*\(/, "", args)
  gsub(/\).*/, "", args)
  gsub(/( *,|^)[^,]*[ *]+/, ", ", args)
  gsub(/\[[0-9]*\]/, "", args)
  gsub(/^, /,"",args)
  if (args == "void") args = ""
  print "  return " name "("args");"
  print "}"
  if (name in CONDITIONALS) print "#endif"
}
