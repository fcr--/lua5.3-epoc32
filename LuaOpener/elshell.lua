-- Lua shell for EPOC
-- written using OPLua
-- (c) Reuben Thomas 2001-2009
-- (c) Francisco Castro 2017

require 'extra'

-- Line input, where params is a table with the following keys:
--   prompt = string with the prompt text (default "> ")
--   line = text to start the input with (default "")
--   point = cursor position within the text (default #line+1)
--   history = list with the history lines.
--   tab = called when the user presses tab, can be used for competion!
local function _input(params)
  local history = params.history or {}
  local s = {
    line = params.line or "",
    point = math.floor(params.point or #(params.line or "") + 1),
    hpos = #history + 1,
    cancelled = false,
    tabmsg = "",
    newline = "" -- saved here to avoid modification of history
  }
  s.point = math.max(1, math.min(s.point, #s.line + 1))
  local termw, termh = opl.rawscreeninfo(3), opl.rawscreeninfo(4)
  local handlers = {
    move = function(pos)
        s.point = math.max(1, math.min(pos, #s.line + 1))
      end,
    insert = function(text)
        s.line = s.line:sub(1, s.point - 1) .. text .. s.line:sub(s.point)
        s.point = s.point + #text
      end,
    linepre = function()
        return s.line:sub(1, s.point - 1)
      end,
    linepost = function()
        return s.line:sub(s.point)
      end,
    tabmsg = function(msg)
        s.tabmsg = msg:gsub("\n", "")
      end,
    cancel = function()
        s.cancelled = true
      end,
    termsize = function()
        return termw, termh
      end,
  }
  local act = {
    [8] = function(s)
        s.line = s.line:sub(1, math.max(0, s.point - 2)) .. s.line:sub(s.point)
        s.point = math.max (s.point - 1, 1)
      end,
    [9] = params.tab and function(s) -- tab
        params.tab(handlers)
      end,
    [27] = function(s) -- esc
        s.cancelled = true
      end,
    [256] = function(s) -- up
        if history[s.hpos - 1] then
          if s.hpos > #history then s.newline = s.line end
          s.line = history[s.hpos - 1]
          s.point = #s.line + 1
          s.hpos = s.hpos - 1
        end
      end,
    [257] = function(s) -- down,
        if history[s.hpos + 1] then
          s.line = history[s.hpos + 1]
          s.point = #s.line + 1
          s.hpos = s.hpos + 1
        elseif s.hpos == #history then -- recover newline
          s.line = s.newline
          s.point = #s.line + 1
          s.hpos = s.hpos + 1
        end
      end,
    [258] = function(s) -- right
        s.point = math.min(s.point + 1, #s.line + 1)
      end,
    [259] = function(s) -- left
        s.point = math.max(s.point - 1, 1)
      end,
    [262] = function(s) -- home
        s.point = 1
      end,
    [263] = function(s) -- end
        s.point = #s.line + 1
      end,
  }
  if opl.rawscreeninfo(5) ~= 0 then print() end
  io.write(params.prompt)
  local c
  local olen, len = 0
  local yinit = opl.rawscreeninfo(6) + 1
  local yfinal, oyfinal = yinit, yinit
  repeat
    c = opl.get()
    --print(c) -- uncomment to add new keys
    if act[c] then
      act[c](s)
    elseif c >= string.byte(" ") and c <= 255 then
      handlers.insert(string.char(c))
    end
    opl.at(1, yinit)
    io.write(params.prompt .. s.line)
    yfinal = opl.rawscreeninfo(6) + 1
    len = #params.prompt + #s.line
    if yfinal > oyfinal then
      s.tabmsg = ''
      oyfinal = yfinal
    elseif s.tabmsg ~= "" then
      local padding = (-len) % termw
      io.write((" "):rep(padding) .. s.tabmsg)
      oyfinal = yfinal
      yfinal = opl.rawscreeninfo(6) + 1
      len = len + padding + #s.tabmsg
    end
    if yfinal == termh then -- it the cursor gets to the bottom right corner
      io.write " " -- the terminal does not scrolls to the following line,
    end -- so we force that by printing a mostly harmless space.
    local nlines = math.floor(len / termw)
    yinit = yfinal - nlines
    if len < olen then
      io.write(string.rep(" ", olen - len))
    end
    olen = len
    opl.at(1 + (s.point + #params.prompt - 1) % termw,
        yinit + math.floor((#params.prompt + s.point - 1) / termw))
  until c == string.byte ("\r") or s.cancelled
  opl.at(1, yfinal)
  io.write ("\n")
  if not s.cancelled then return s.line end
end

-- Set _ALERT
_ALERT = opl.alert
if not history then
  history = {}
end

-- Main loop
local function filterError(e)
  return (e:gsub("^%[string.-:1: ", ""))
end
local function handle(res, ...)
  local arg1 = ...
  if not res then
    if type(arg1) == "string" then
      print(filterError(arg1), select(2, ...))
    else
      print(...)
    end
    return nil
  end
  return ...
end
local function common_prefix(a, b) -- ('abcd', 'abxy') -> 'ab'
  for i = 1, #a do
    if a:byte(i) ~= b:byte(i) then return a:sub(1, i-1) end
  end
  return a
end
local completion_suffixes = {
  ["function"] = "(",
  ["table"] = "."
}
local function ontab(handlers)
  handlers.tabmsg('')
  local words = handlers.linepre():match"[%a%d_:.]+$"
  if not words then return end
  words = words:split"%."
  local t, word = _G, words[#words]
  for i = 1, #words-1 do
    t = t[words[i]]
    if type(t) ~= "table" then return end
  end
  local gcp -- greatest common prefix
  local options = {}
  for k, v in pairs(t) do -- now we try to complete the last word
    if type(k) == "string" and k:sub(1, #word) == word then
      local extra = k:sub(#word+1) -- k is candidate
      if completion_suffixes[type(v)] then
        extra = extra .. completion_suffixes[type(v)]
      end
      gcp = gcp and common_prefix(gcp, extra) or extra
      table.insert(options, k)
    end
  end
  if gcp and gcp ~= "" then
    handlers.insert(gcp)
  elseif #options > 1 then
    table.sort(options)
    local tw,th = handlers.termsize()
    handlers.tabmsg(table.concat(options, ' '):sub(1, tw*10))
  end
end
opl.cursoron()
repeat
  local s = _input{history = history, prompt = "> ", tab = ontab}
  if s == "q" or s == "quit" or s == "exit" then
    break
  elseif s == "r" or s == "reload" then
    return dofile(debug.getinfo(1).source:sub(2))
  elseif s and s ~= "" then
    if s:sub(1, 1) == "=" then
      s = "return " .. s:sub(2)
    end
    table.insert(history, s)
    local f = load("return "..s)
    if f == nil then
      f = handle(pcall(assert, load(s)))
    end
    if type(f) == "function" then
      local res = {handle(pcall(f))}
      for i=1,#res do if res[i] == nil then res[i]="nil" end end
      if #res > 0 then print(table.unpack(res)) end
    end
  end
until false
os.exit()

-- Changelog

-- 05jan09 Put main loop at top level, not in _main function
-- 04jan09 Correct key code for down arrow
-- 03nov07 Use alert for _ALERT
-- 02jun07 Split out OPL bindings from shell code
-- 12mar04 Filter error messages to make them neater
-- 11mar04 Added beginning of cursor movement to line editor
--         Make reloading the shell into a tail call
--         Sorted out error handling for parsing vs execution
-- 06mar04 Added _oplinput
--         Debugged io.write override
--         Fixed print override for nil values
--         Added _shell (reorganised so that the main loop is in Lua)
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
-- 14mar01 File updated (still no code!)
-- 01feb01 File started (no code)
