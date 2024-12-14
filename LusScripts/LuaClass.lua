local playerModel = glueLoadModelResource(OBJECT.CUBE)

local m = Model.new()

print("a")

m:setgltfModel(playerModel)
glueBindCamera(m)

print("b")

glueAddModel(m)