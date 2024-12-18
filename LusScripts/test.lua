local model = glueCreateModel()

model:setGltfModel()

local mt = debug.getmetatable(model)
if mt and mt.setGltfModel then
    print("a")
    mt.setGltfModel(model)
else
    print("b")
end