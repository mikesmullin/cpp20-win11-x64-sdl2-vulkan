print("[Lua] pong script loading.")

AddSoundEffect("../assets/audio/sfx/mop2.ogg")

local angle = 0
local ROT_SPEED = 30
local pressed = false
local MOVE_SPEED = 2.0 -- per sec
local x, y, z = 0.0, 1.0, 2.0
function OnUpdate(deltaTime)
  --angle = angle + (ROT_SPEED * deltaTime)
  --while angle > 360 do
  --angle = angle - 360
  --end
  --  while angle < 0 do
  --    angle = angle + 360
  --  end

  local x1, y1, x2, y2, b1, b2, b3, b4 = GetGamepadInput(0)
  if b1 and not pressed then
    pressed = true
    print("button1")
    PlaySoundEffect(0)
  elseif not b1 and pressed then
    pressed = false
  end

  x = x + (x1 * MOVE_SPEED * deltaTime);
  y = y + (-y1 * MOVE_SPEED * deltaTime);
  z = z + (-y2 * MOVE_SPEED * deltaTime);

  return angle, x, y, z
end

print("[Lua] pong script done loading.")
