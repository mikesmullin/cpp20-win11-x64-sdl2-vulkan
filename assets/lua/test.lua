print("hello world from lua!")

local paddle1 = RegisterEntity()
print("[Lua] Got paddle1 eid = " .. paddle1 .. ".")
local paddle2 = RegisterEntity()
print("[Lua] Got paddle2 eid = " .. paddle2 .. ".")

local transformable = RegisterComponent(function()
  print("[Lua] transformable Component::OnUpdate() called.")
end)

print("[Lua] end of script reached.")

--a = 7 + 11 + math.sin(23.7)
