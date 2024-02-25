print("[Lua] pong script loading.")

-- external interface
---@class _G
---@field package LoadTexture fun(file: string): nil
---@field package LoadAudioFile fun(file: string): nil
---@field package LoadShader fun(file: string): nil
---@field package PlayAudio fun(id: number, loop: boolean, gain: number): nil
---@field package AddInstance fun(): number
---@field package GetGamepadInput fun(id: number): number, number, number, number, boolean, boolean, boolean, boolean
---@field package GetKeyboardInput fun(): boolean, boolean, boolean, boolean, boolean, number, number
---@field package ReadInstanceVBO fun(id: number): number, number, number, number, number, number, number, number, number, number
---@field package WriteInstanceVBO fun(id: number, posX: number, posY: number, posZ: number, rotX: number, rotY: number, rotZ: number, scaleX: number, scaleY: number, scaleZ: number, texId: number): nil
---@field package WriteWorldUBO fun(aspect: number, camX: number, camY: number, camZ: number, lookX: number, lookY: number, lookZ: number, user1X: number, user1Y: number, user2X: number, user2Y: number): nil
---@field package Exit fun(): nil

-- internal OOP

local ASPECT_16_9 = 16 / 9 -- Widescreen
local ASPECT_4_3 = 4 / 3   -- CRT
local ASPECT_1_1 = 1 / 1   -- Square

---@class World
---@field public aspect number
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
  self.aspect = ASPECT_1_1
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

function World:set(aspect, camX, camY, camZ, lookX, lookY, lookZ)
  self.aspect = aspect
  self.camX = camX
  self.camY = camY
  self.camZ = camZ
  self.lookX = lookX
  self.lookY = lookY
  self.lookZ = lookZ
end

function World:push()
  _G.WriteWorldUBO(self.aspect, self.camX, self.camY, self.camZ, self.lookX, self.lookY, self.lookZ, self.user1X,
    self.user1Y,
    self.user2X, self.user2Y)
end

local world = World.new()

---@class Instance
---@field public id number
---@field public posX number
---@field public posY number
---@field public posZ number
---@field public rotX number
---@field public rotY number
---@field public rotZ number
---@field public scaleX number
---@field public scaleY number
---@field public scaleZ number
---@field public texId number
local Instance = {}
Instance.__index = Instance
function Instance.new()
  local self = setmetatable({}, Instance)
  self.id = _G.AddInstance()
  self.posX = 0
  self.posY = 0
  self.posZ = 0
  self.rotX = 0
  self.rotY = 0
  self.rotZ = 0
  self.scaleX = 1
  self.scaleY = 1
  self.scaleZ = 1
  self.texId = 0
  return self
end

function Instance:push()
  _G.WriteInstanceVBO(self.id, self.posX, self.posY, self.posZ, self.rotX, self.rotY, self.rotZ, self.scaleX, self
    .scaleY, self.scaleZ, self.texId)
end

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
_G.PlayAudio(0, true, 2.0)

-- position the camera
world:set(ASPECT_1_1, 0, 0, 1, 0, 0, 0)
world:push()

-- put entities on screen

local BACKGROUND_WH = 800
local background = Instance.new()
background.texId = 0
background:push()

local PIXELS_PER_UNIT = BACKGROUND_WH
function PixelsToUnits(pixels)
  return pixels / PIXELS_PER_UNIT
end

-- render some pixel font glyphs

local GLYPH_W = 4
local GLYPH_H = 6
local GLYPH_SCALE = 4
function CreateGlyphs(x, y, scale, txt)
  local glyphs = {}
  local t
  local char = ""
  local code = 0
  for i = 1, #txt do
    local char = string.sub(txt, i, i)
    local code = string.byte(char)

    t = Instance.new()
    t.posX = PixelsToUnits(x + (GLYPH_W * scale * (i - 1)))
    t.posY = PixelsToUnits(y)
    t.scaleX = PixelsToUnits(GLYPH_W * scale)
    t.scaleY = PixelsToUnits(GLYPH_H * scale)
    t.texId = code
    t:push()

    table.insert(glyphs, t)
  end
  return glyphs
end

local OFFSET_X = ((-BACKGROUND_WH + (GLYPH_W * GLYPH_SCALE * 2)) / 2)
local OFFSET_Y = ((-BACKGROUND_WH + (GLYPH_H * GLYPH_SCALE * 2)) / 2)
local txtScore = CreateGlyphs(OFFSET_X, OFFSET_Y, GLYPH_SCALE, "Score: 0  ")

---@class Rigidbody
---@field public inst Instance
---@field public vx number
---@field public vy number
local Rigidbody = {}
Rigidbody.__index = Rigidbody
function Rigidbody.new(inst, vx, vy)
  local self = setmetatable({}, Rigidbody)
  self.inst = inst
  self.vx = vx or 0 -- Velocity along x-axis
  self.vy = vy or 0 -- Velocity along y-axis
  return self
end

-- Update the position of the rigid body based on its velocity
---@param dt number
function Rigidbody:update(dt)
  self.inst.posX = self.inst.posX + self.vx * dt
  self.inst.posY = self.inst.posY + self.vy * dt
end

---@class BoxCollider2d
---@field public rb Rigidbody
---@field public width number
---@field public height number
local BoxCollider2d = {}
BoxCollider2d.__index = BoxCollider2d
function BoxCollider2d.new(rb, width, height)
  local self = setmetatable({}, BoxCollider2d)
  self.rb = rb
  self.width = width or 0
  self.height = height or 0
  return self
end

-- Check collision between two rectangles
---@param other BoxCollider2d
function BoxCollider2d:checkCollision(other)
  local r1x = self.rb.inst.posX - self.width / 2
  local r1y = self.rb.inst.posY - self.height / 2
  local r2x = other.rb.inst.posX - other.width / 2
  local r2y = other.rb.inst.posY - other.height / 2

  local outcome = r1x + self.width >= r2x and -- r1 right edge past r2 left
      r1x <= r2x + other.width and            -- r1 left edge past r2 right
      r1y + self.height >= r2y and            -- r1 top edge past r2 bottom
      r1y <= r2y + other.height               -- r1 bottom edge past r2 top

  return outcome
end

local PADDLE_START_Y = PixelsToUnits(720) / 2
local PADDLE_W = PixelsToUnits(170)
local PADDLE_H = PixelsToUnits(45)
local PADDLE_SPEED = 1.0 -- per sec
local PADDLE_SPEED_INC = 1.0 / 10
local PADDLE_BOUNDS_X = PixelsToUnits(BACKGROUND_WH / 2)

local paddle = Instance.new()
-- NOTICE: coordinate system is 0,0 == center of screen
-- TODO: do I want 0,0 to be in a corner, instead?
paddle.posX = 0
paddle.posY = PADDLE_START_Y
paddle.scaleX = PADDLE_W
paddle.scaleY = PADDLE_H
paddle.texId = 1

---@type Rigidbody
local paddle_rb = Rigidbody.new(paddle, 0, 0)
---@type BoxCollider2d
local paddle_collider = BoxCollider2d.new(paddle_rb, PADDLE_W, PADDLE_H)

paddle:push()

local BALL_START_Y = PixelsToUnits(100)
local BALL_SIZE_WH = PixelsToUnits(45)
local BALL_SPEED = 1.0 -- per sec
local BALL_SPEED_INC = 1.0 / 50
local BALL_BOUNDS_X = PixelsToUnits(BACKGROUND_WH / 2)
local BALL_BOUNDS_Y = PixelsToUnits(BACKGROUND_WH / 2)

local ball = Instance.new()
ball.scaleX = BALL_SIZE_WH
ball.scaleY = BALL_SIZE_WH
ball.texId = 2

---@type Rigidbody
local ball_rb = Rigidbody.new(ball, 0, 0)
---@type BoxCollider2d
local ball_collider = BoxCollider2d.new(ball_rb, BALL_SIZE_WH, BALL_SIZE_WH)

---@param rb Rigidbody
function Ball__Reset(rb)
  rb.inst.posX = 0
  rb.inst.posY = BALL_START_Y / 2
  rb.vx = BALL_SPEED / 2
  rb.vy = BALL_SPEED
end

Ball__Reset(ball_rb)

ball:push()

-- helper functions
function FixJoyDrift(x)
  if x > -0.1 and x < 0.1 then return 0 else return x end
end

function Math__clamp(value, min, max)
  return math.min(math.max(value, min), max)
end

-- main loop

local score = 0
local State = {
  PLAYING = 0,
  PAUSED = 1,
  SCORE = 2,
}
local gameState = State.PAUSED

---@param idx number
---@param txt number
---@return number
function GetCharCodeAt(idx, txt)
  local str = tostring(txt)
  if idx > string.len(txt) then return 32 end
  local char = string.sub(str, idx, idx)
  local code = string.byte(char)
  return code
end

---@param arr table
---@param idx number
---@param code number
function UpdateGlyph(arr, idx, code)
  if code > 31 and code < 128 then
    arr[idx].texId = code
    arr[idx]:push()
  end
end

---@param score number
function UpdateScore(score)
  UpdateGlyph(txtScore, 8, GetCharCodeAt(1, score))
  UpdateGlyph(txtScore, 9, GetCharCodeAt(2, score))
  UpdateGlyph(txtScore, 10, GetCharCodeAt(3, score))
end

local ball_in_collission = false
local on_ball_hit_paddle = false
function OnFixedUpdate(deltaTime)
  if gameState ~= State.PLAYING then return end

  local colliding = ball_collider:checkCollision(paddle_collider)
  if colliding and ball_in_collission then
    on_ball_hit_paddle = false
    ball_in_collission = true
  elseif not colliding and ball_in_collission then
    on_ball_hit_paddle = false
    ball_in_collission = false
  elseif colliding and not ball_in_collission then
    on_ball_hit_paddle = true
    ball_in_collission = true
  elseif not colliding and not ball_in_collission then
    on_ball_hit_paddle = false
    ball_in_collission = false
  end

  -- physics moving ball X,Y
  ball_rb:update(deltaTime)

  -- ball bounce off top wall
  if ball.posY <= -BALL_BOUNDS_Y then
    ball_rb.vy = math.abs(ball_rb.vy)

    -- ball collision w paddle
  elseif on_ball_hit_paddle then
    -- play one-shot sound effect
    _G.PlayAudio(math.random(1, 15), false, 0.5)

    score = score + 1
    UpdateScore(score)

    ball_rb.vy = -math.abs(ball_rb.vy)

    -- incease ball velocity with each hit
    ball_rb.vx = ball_rb.vx + (ball_rb.vx * BALL_SPEED_INC)
    ball_rb.vy = ball_rb.vy + (ball_rb.vy * BALL_SPEED_INC)

    -- ball missed paddle
  elseif ball.posY >= BALL_BOUNDS_Y then
    gameState = State.SCORE
  end

  -- ball collision with side walls
  if ball.posX >= BALL_BOUNDS_X then
    ball_rb.vx = -math.abs(ball_rb.vx)
  elseif ball.posX <= -BALL_BOUNDS_X then
    ball_rb.vx = math.abs(ball_rb.vx)
  end

  ball:push()
end

function b(v)
  return v and "true" or "false"
end

local x1, y1, x2, y2 = 0, 0, 0, 0
local b1, b2, b3, b4 = false, false, false, false
local pressed = false
local code, location = 0, 0
local keyPressed, alt, ctrl, shift, meta = false, false, false, false, false
local keyState = false
local left1, right1, left2, right2 = false, false, false, false
local kbXAxis, xAxis = 0.0, 0.0
local x = 0

function OnUpdate(deltaTime)
  -- read gamepad input
  x1, y1, x2, y2, b1, b2, b3, b4 = _G.GetGamepadInput(0)
  -- on button press
  if b1 and not pressed then
    pressed = true
  elseif not b1 and pressed then
    pressed = false
  end

  --if gameState ~= State.PLAYING then return end

  xAxis = 0.0

  -- read keyboard input
  keyPressed, alt, ctrl, shift, meta, code, location = _G.GetKeyboardInput()
  if keyPressed and not keyState then -- KEYDOWN
    keyState = true

    if code == 26 then     -- W
    elseif code == 82 then -- UP
    elseif code == 4 then  -- A
      left1 = true
    elseif code == 80 then -- LEFT
      left2 = true
    elseif code == 22 then -- S
    elseif code == 81 then -- DOWN
    elseif code == 7 then  -- D
      right1 = true
    elseif code == 79 then -- RIGHT
      right2 = true
    elseif code == 44 then -- SPACE
      pressed = true
    elseif code == 41 then -- ESC
      _G.Exit()
    end

    -- print("[Lua] keydown " ..
    --   "code " .. code ..
    --   " location " .. location ..
    --   " alt " .. b(alt) ..
    --   " ctrl " .. b(ctrl) ..
    --   " shift " .. b(shift) ..
    --   " meta " .. b(meta))
  elseif not keyPressed and keyState then -- KEYUP
    keyState = false

    if code == 26 then     -- W
    elseif code == 82 then -- UP
    elseif code == 4 then  -- A
      left1 = false
    elseif code == 80 then -- LEFT
      left2 = false
    elseif code == 22 then -- S
    elseif code == 81 then -- DOWN
    elseif code == 7 then  -- D
      right1 = false
    elseif code == 79 then -- RIGHT
      right2 = false
    elseif code == 44 then -- SPACE
      pressed = false
    end
  end

  kbXAxis = 0.0
  if ((left1 or left2) and (right1 or right2)) then
    kbXAxis = 0.0
  elseif (left1 or left2) and (not (right1 or right2)) then
    kbXAxis = -1.0
  elseif (not (left1 or left2)) and (right1 or right2) then
    kbXAxis = 1.0
  end

  if kbXAxis ~= 0.0 then
    xAxis = kbXAxis
  elseif x1 ~= 0.0 then
    xAxis = x1
  end

  if pressed then
    if gameState == State.SCORE then
      gameState = State.PAUSED
      score = 0
      UpdateScore(score)
      Ball__Reset(ball_rb)
      ball:push()
    elseif gameState == State.PAUSED then
      gameState = State.PLAYING
    elseif gameState == State.PLAYING then
      gameState = State.PAUSED
    end
  end

  -- apply joystick movement over time
  x = (FixJoyDrift(xAxis) * math.abs(ball_rb.vx) * deltaTime)

  if x ~= 0 then
    -- player moving paddle X
    paddle.posX = Math__clamp(paddle.posX + x, -PADDLE_BOUNDS_X, PADDLE_BOUNDS_X)
    paddle:push()
  end
end

print("[Lua] pong script done loading.")
