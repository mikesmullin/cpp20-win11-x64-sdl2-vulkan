print("hello world from lua!")

function AddStuff(a, b)
  print("[Lua] AddStuff(" .. a .. ", " .. b .. ") called.")
  return a + b
end

local paddle1 = RegisterEntity()
print("[Lua] Got paddle1 eid = " .. paddle1 .. ".")
local paddle2 = RegisterEntity()
print("[Lua] Got paddle2 eid = " .. paddle2 .. ".")

--a = 7 + 11 + math.sin(23.7)
