print("[Lua] pong script loading.")

LoadTexture("../assets/textures/crates.png")
LoadSoundEffect("../assets/audio/sfx/mop2.ogg")
LoadShader("../assets/shaders/simple_shader.frag.spv")
LoadShader("../assets/shaders/simple_shader.vert.spv")

for i = 0, 25 do
  AddInstance()
end

local angle = 0
local ROT_SPEED = 30
local pressed = false
local MOVE_SPEED = 2.0 -- per sec
local x, y, z = 0.0, 0.0, 0.0
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
    --print("button1")
    AddInstance()
    PlaySoundEffect(0)
    AdjustVBO();
  elseif not b1 and pressed then
    pressed = false
  end

  function FixJoyDrift(x)
    if x > -0.1 and x < 0.1 then return 0 else return x end
  end

  x = (FixJoyDrift(x1) * MOVE_SPEED * deltaTime);
  y = (FixJoyDrift(-y1) * MOVE_SPEED * deltaTime);
  z = (FixJoyDrift(-y2) * MOVE_SPEED * deltaTime);

  return angle, x, y, z
end

print("[Lua] pong script done loading.")
