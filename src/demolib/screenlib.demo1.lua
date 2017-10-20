-- opl.defaultwin(1) -- 4 colors + dithering

local s = require "screenlib"

function circle(centerx, centery, radius)
  local centerx, centery = math.floor(centerx), math.floor(centery)
  s.setpensize(3, 3)
  for a = -3.14, 3.15, 0.02 do
    local dx = math.floor(math.cos(a) * radius)
    local dy = math.floor(math.sin(a) * radius)
    local c = math.floor((a+3.14)/6.29*256*6) % 512
    if c > 255 then c = 511 - c end
    s.setpencolor(c, c, c)
    s.moveto(centerx, centery)
    s.drawlineby(dx, dy)
  end
end

repeat
  opl.at(1,1)
  print [[DEMO:
    0. exit
    1. circle
    2. linear gradient]]
  local n = tonumber(io.read())
  if n == 1 then
    circle(320, 120, 100)
  elseif n == 2 then
    s.setpensize(1,1)
    for x = 0, 639 do
      local c = x // 0x28 * 0x11
      s.setpencolor(c, c, c)
      s.moveto(x, 0); s.drawlineby(0, 240)
    end
  end
until n == 0
