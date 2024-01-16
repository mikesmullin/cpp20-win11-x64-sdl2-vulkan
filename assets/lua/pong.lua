print("[Lua] pong script loading.")

AddSoundEffect("../assets/audio/sfx/mop2.ogg")
PlaySoundEffect(0)

local angle = 0
local ROT_SPEED = 0.05
function SetRotAngle(deltaTime)
  angle = angle + (ROT_SPEED * deltaTime)

  local x1, y1, x2, y2, b1, b2, b3, b4 = GetGamepadInput(0)
  if (b1) then
    print("button1")
  end
  return angle
end

print("[Lua] pong script done loading.")
