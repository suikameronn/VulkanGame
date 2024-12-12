--[[
Object = {}

Object.type = 0
Object.position = {x = 0.0,y = 0.0,z = 0.0}

Object.childrenObjects = {}

Object.setPosition = function(x,y,z)
    Object.position.x = x
    Object.position.y = y
    Object.position.z = z
end
Object.setLuaScript = function(luaPath)
    Object.luaPath = luaPath
end
Object.setChildren = function(obj)
    table.insert(Object.childrenObjects,obj)
end

Model = {}
Model.type = 1
Model.objResource = "CUBE"
Model.scale = {x = 1.0,y = 1.0,z = 1.0}

Model.setColider = function()
    Model.colider = true
end
Model.bindCamera = function()
    Model.camera = true
end
Model.new = function()
    local obj = {}
    setmetatable(obj,{__index=Object})
    return obj
end
]]--

local m = Object.new()

glueAddObject(m)