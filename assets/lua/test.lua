print("hello world from lua!")

---@class _G
---@field package RegisterEntity fun(): number
---@field package RegisterComponent fun(cb: fun(): nil): nil

local paddle1 = _G.RegisterEntity()
print("[Lua] Got paddle1 eid = " .. paddle1 .. ".")
local paddle2 = _G.RegisterEntity()
print("[Lua] Got paddle2 eid = " .. paddle2 .. ".")

local transformable = _G.RegisterComponent(function()
  print("[Lua] transformable Component::OnUpdate() called.")
end)

print("[Lua] end of script reached.")

--a = 7 + 11 + math.sin(23.7)
