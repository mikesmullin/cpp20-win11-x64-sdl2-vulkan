print("[Lua] pong script loading.")

_G.LoadTexture("../assets/textures/pong-atlas.png")
_G.LoadAudioFile("../assets/audio/music/retro.wav")
_G.LoadAudioFile("../assets/audio/sfx/pong-01.wav")
_G.LoadAudioFile("../assets/audio/sfx/pong-02.wav")
_G.LoadAudioFile("../assets/audio/sfx/pong-03.wav")
_G.LoadAudioFile("../assets/audio/sfx/pong-04.wav")
_G.LoadAudioFile("../assets/audio/sfx/pong-05.wav")
_G.LoadAudioFile("../assets/audio/sfx/pong-06.wav")
_G.LoadAudioFile("../assets/audio/sfx/pong-07.wav")
_G.LoadAudioFile("../assets/audio/sfx/pong-08.wav")
_G.LoadAudioFile("../assets/audio/sfx/pong-09.wav")
_G.LoadAudioFile("../assets/audio/sfx/pong-10.wav")
_G.LoadAudioFile("../assets/audio/sfx/pong-11.wav")
_G.LoadAudioFile("../assets/audio/sfx/pong-12.wav")
_G.LoadAudioFile("../assets/audio/sfx/pong-13.wav")
_G.LoadAudioFile("../assets/audio/sfx/pong-14.wav")
_G.LoadAudioFile("../assets/audio/sfx/pong-15.wav")
_G.LoadShader("../assets/shaders/simple_shader.frag.spv")
_G.LoadShader("../assets/shaders/simple_shader.vert.spv")

_G.PlayAudio(0, true)

for i = 0, 25 do
  _G.AddInstance()
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

  local x1, y1, x2, y2, b1, b2, b3, b4 = _G.GetGamepadInput(0)
  if b1 and not pressed then
    pressed = true
    --print("button1")
    _G.AddInstance()
    _G.PlayAudio(math.random(1, 15), false)
    _G.AdjustVBO();
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
