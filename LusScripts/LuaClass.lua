

Object = {};

Object.position.x = 0.0;
Object.position.y = 0.0;
Object.position.z = 0.0;

Object.luaPath = "noScript"

Object.childrenObjects = {};

Object.setPosition = function(x,y,z)
    Object.position.x = x;
    Object.position.y = y;
    Object.position.z = z;
end
Object.setLuaScript = function(luaPath)
    Object.luaPath = luaPath;
end
Object.setChildren = function(obj)
    table.insert(Object.childrenObjects,obj)
end

Model = {};
Model.objType = "none"