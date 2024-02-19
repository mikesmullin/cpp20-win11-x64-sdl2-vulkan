print("[Lua] pong script loading.")

-- external interface
---@class _G
---@field package LoadTexture fun(file: string): nil
---@field package LoadAudioFile fun(file: string): nil
---@field package LoadShader fun(file: string): nil
---@field package PlayAudio fun(id: number, loop: boolean): nil
---@field package AddInstance fun(): number
---@field package GetGamepadInput fun(id: number): number, number, number, number, boolean, boolean, boolean, boolean
---@field package ReadInstanceVBO fun(id: number): number, number, number, number, number, number, number, number
---@field package WriteInstanceVBO fun(id: number, posX: number, posY: number, posZ: number, rotX: number, rotY: number, rotZ: number, scale: number, texId: number): nil
---@field package WriteWorldUBO fun(camX: number, camY: number, camZ: number, lookX: number, lookY: number, lookZ: number): nil

-- internal OOP

---@class World
---@field public camX number
---@field public camY number
---@field public camZ number
---@field public lookX number
---@field public lookY number
---@field public lookZ number
local World = {}
World.__index = World
function World.new()
  local self = setmetatable({}, World)
  self.camX = 0
  self.camY = 0
  self.camZ = 0
  self.lookX = 0
  self.lookY = 0
  self.lookZ = 0
  return self
end

function World:set(camX, camY, camZ, lookX, lookY, lookZ)
  self.camX = camX
  self.camY = camY
  self.camZ = camZ
  self.lookX = lookX
  self.lookY = lookY
  self.lookZ = lookZ
end

function World:push()
  _G.WriteWorldUBO(self.camX, self.camY, self.camZ, self.lookX, self.lookY, self.lookZ)
end

local world = World.new()

-- preload assets
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

-- play music on loop
_G.PlayAudio(0, true)

-- position the camera
world:set(1, 3, 5, 0, 0, 0)
world:push()

-- put three entities on screen
local background = _G.AddInstance()
_G.WriteInstanceVBO(background, 0, 0, 0, 0, 0, 0, 1, 0)
local paddle = _G.AddInstance()
_G.WriteInstanceVBO(paddle, 0, 1, 0, 0, 0, 0, 1, 1)
local ball = _G.AddInstance()
_G.WriteInstanceVBO(ball, 1, 1, 0, 0, 0, 0, 1, 2)

-- helper functions
function FixJoyDrift(x)
  if x > -0.1 and x < 0.1 then return 0 else return x end
end

-- main draw loop
local pressed = false
local MOVE_SPEED = 2.0 -- per sec
local x, y, z = 0.0, 0.0, 0.0
function OnUpdate(deltaTime)
  -- read gamepad input
  local x1, y1, x2, y2, b1, b2, b3, b4 = _G.GetGamepadInput(0)
  -- on button press
  if b1 and not pressed then
    pressed = true
    -- play one-shot sound effect
    _G.PlayAudio(math.random(1, 15), false)
  elseif not b1 and pressed then
    pressed = false
  end

  -- apply joystick movement over time
  x = (FixJoyDrift(x1) * MOVE_SPEED * deltaTime)
  y = (FixJoyDrift(-y1) * MOVE_SPEED * deltaTime)
  z = (FixJoyDrift(-y2) * MOVE_SPEED * deltaTime)

  if x ~= 0 or y ~= 0 or z ~= 0 then
    -- by moving camera
    world.camX = world.camX + x
    world.camY = world.camY + y
    world.camZ = world.camZ + z
    print("[Lua] world.cam x " .. world.camX .. " y " .. world.camY .. " z " .. world.camZ)
    world:push()
  end

  --local px, py, pz, rx, ry, rz, scale, texId = _G.ReadInstanceVBO(paddle)
  --_G.WriteInstanceVBO(paddle, x, y, z, 0, 0, 0, 1, 1)
end

print("[Lua] pong script done loading.")
