print("[Lua] pong script loading.")

local angle = 0
local ROT_SPEED = 0.05
function SetRotAngle(deltaTime)
  angle = angle + (ROT_SPEED * deltaTime)
  return angle
end

print("[Lua] pong script done loading.")
