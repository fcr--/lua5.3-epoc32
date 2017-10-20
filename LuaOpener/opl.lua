-- OPL bindings for Lua
-- (c) Reuben Thomas 2001-2009

local _luaregister = opl._luaregister
opl._luaregister = nil
function opl.luaregister(declaration)
  _luaregister(opl._luastate, declaration, '_luaregister.tmp')
  local symbol = _G['_luaregister.tmp']
  _G['_luaregister.tmp'] = nil
  return symbol
end

for name, declaration in pairs {
    eval      = "Eval:(exp$)",
    get       = "Get%:",
    cursoron  = "CursorOn:",
    cursoroff = "CursorOff:",
    at        = "At:(x%,y%)",
    rawscreeninfo = "RawScreenInfo%:(pos%)",
    print     = "OplPrint:(s$)",
    input     = "OplInput$:",
    alert     = "OplAlert:(m$)",
    loadm     = "OplLoadM:(m$)",
    unloadm   = "OplUnloadM:(m$)",
    defaultwin = "OplDefaultWin:(m%)"} do
  opl[name] = opl.luaregister(declaration)
end

-- Redefine Lua I/O functions

-- chunk: Call f with 255-character chunks of each tostring (arg[i])
--   f: function to call
--   ...: objects
local function chunk(f, ...)
  for _, v in ipairs{...} do
    local s = tostring(v):gsub("\r?\n", "\r\n")
    local n, len = 1, #s
    while n <= len - 255 do
      f(string.sub(s, n, n + 254))
      n = n + 255
    end
    f(string.sub(s, n))
  end
end

-- Override print
function print(...)
  local arg = {...}
  for i, v in ipairs(arg) do
    chunk(opl.print, tostring(v))
    if i < #arg then
      opl.print "\t"
    end
  end
  opl.print "\r\n"
end

-- Override io.read
local buffer
function io.read(p)
  if p == "*line" or p == nil then
    if not buffer then return opl.input() end
    local res = buffer
    buffer = nil
    return res
  elseif type(p) == "number" then
    local res = {}
    while p > 0 do
      if not buffer then
        buffer = opl.input()
      end
      local blen = #buffer
      if p > blen then
        table.insert(res, buffer)
        table.insert(res, "\n")
        buffer = nil
      else
        res[#res + 1] = (buffer.."\n"):sub(1, p)
        buffer = buffer:sub(1+p)
      end
      p = p - blen - 1
    end
    return table.concat(res)
  elseif p == "*number" then
    if not buffer then
      buffer = opl.input()
    end
    local n, r, e = buffer:match "^%s([%+%-]?0x[0-9a-fA-F]+)(.*)"
    if n then buffer=r; return tonumber(n) end
    n, e, r = buffer:match "^%s([%+%-]?[0-9]+%.?[0-9]*)(e?[%+%-]?[0-9]*)(.*)"
    if n and e:match "e.*[0-9]" then buffer=r; return tonumber(n) end
    n, e, r = buffer:match "^%s([%+%-]?%.?[0-9]+)(e?[%+%-]?[0-9]*)(.*)"
    if n and e:match "e.*[0-9]" then buffer=r; return tonumber(n) end
  elseif p == "*all" then
    return (buffer and (buffer.."\n") or "") .. io.input():read "*all"
  else
    error("Ilegal argument for read.")
  end
end

-- Override io.write
function io.write(...)
  chunk(opl.print, ...)
end

-- Changelog

-- 25sep17 Fix all the wrappers
-- 03nov07 Add ALERT binding via OplAlert:
-- 02jun07 Split out OPL bindings from shell code
-- 06mar04 Added _oplinput
--         Debugged io.write override
--         Fixed print override for nil values
-- 11oct03 Simplify code
-- 08oct03 Override error printing correctly for Lua 5
-- 01jun03 Updated for Lua5
-- 05dec02 Removed apply, which wasn't used
-- 29nov02 Removed spurious arg from write
--         Add newline output to print, as it's now (correctly)
--         removed from _oplprint
-- 01mar02 print implemented using _oplprint
--         Added _oplchunk
--         write implemented using _oplwrite
