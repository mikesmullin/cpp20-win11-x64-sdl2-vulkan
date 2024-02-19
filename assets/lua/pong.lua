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
---@field package WriteWorldUBO fun(camX: number, camY: number, camZ: number, lookX: number, lookY: number, lookZ: number, user1X: number, user1Y: number, user2X: number, user2Y: number): nil

-- internal OOP

---@class World
---@field public camX number
---@field public camY number
---@field public camZ number
---@field public lookX number
---@field public lookY number
---@field public lookZ number
---@field public user1X number
---@field public user1Y number
---@field public user2X number
---@field public user2Y number
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
  self.user1X = 0
  self.user1Y = 0
  self.user2X = 1
  self.user2Y = 1
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
  _G.WriteWorldUBO(self.camX, self.camY, self.camZ, self.lookX, self.lookY, self.lookZ, self.user1X, self.user1Y,
    self.user2X, self.user2Y)
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
-- _G.WriteInstanceVBO(paddle, 0, 1, 0, 0, 0, 0, 1, 1)
-- local ball = _G.AddInstance()
-- _G.WriteInstanceVBO(ball, 1, 1, 0, 0, 0, 0, 1, 2)

-- helper functions
function FixJoyDrift(x)
  if x > -0.1 and x < 0.1 then return 0 else return x end
end

-- main draw loop
local pressed = false
local MOVE_SPEED = 0.25 -- per sec
local SLOW_SPEED = 0.25 -- multiplier
local x, y, z = 0, 0, 0
local u, v, w, h = 0, 0, 0, 0
local slow = false
function OnUpdate(deltaTime)
  -- read gamepad input
  local x1, y1, x2, y2, b1, b2, b3, b4 = _G.GetGamepadInput(0)
  -- on button press
  if b1 and not pressed then
    pressed = true
    -- play one-shot sound effect
    --_G.PlayAudio(math.random(1, 15), false)
    slow = true
  elseif not b1 and pressed then
    pressed = false
    slow = false
  end

  -- apply joystick movement over time
  --x = (FixJoyDrift(x1) * MOVE_SPEED * deltaTime)
  --y = (FixJoyDrift(-y1) * MOVE_SPEED * deltaTime)
  --z = (FixJoyDrift(-y2) * MOVE_SPEED * deltaTime)
  u = (FixJoyDrift(-x1) * MOVE_SPEED * (slow and SLOW_SPEED or 1) * deltaTime)
  v = (FixJoyDrift(-y1) * MOVE_SPEED * (slow and SLOW_SPEED or 1) * deltaTime)
  w = (FixJoyDrift(-x2) * MOVE_SPEED * (slow and SLOW_SPEED or 1) * deltaTime)
  h = (FixJoyDrift(-y2) * MOVE_SPEED * (slow and SLOW_SPEED or 1) * deltaTime)

  --if x ~= 0 or y ~= 0 or z ~= 0 then
  if u ~= 0 or v ~= 0 or w ~= 0 or h ~= 0 then
    -- -- by moving camera
    -- world.camX = world.camX + x
    -- world.camY = world.camY + y
    -- world.camZ = world.camZ + z
    -- print("[Lua] world.cam x " .. world.camX .. " y " .. world.camY .. " z " .. world.camZ)
    -- world:push()

    -- by moving texture UVs
    world.user1X = world.user1X + u
    world.user1Y = world.user1Y + v
    world.user2X = world.user2X + w
    world.user2Y = world.user2Y + h
    print("[Lua] world.user" ..
      " u " .. world.user1X ..
      " v " .. world.user1Y ..
      " w " .. world.user2X ..
      " h " .. world.user2Y)
    world:push()
  end
end

print("[Lua] pong script done loading.")
