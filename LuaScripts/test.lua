
local model = glueCreateModel()
glueSetGltfModel(model,CUBE)
glueSetPos(model,0.0,0.0,0.0)
glueSetScale(model,142.0,1.0,140.0)
glueSetColider(model,false)

local model2 = glueCreateModel()
glueSetGltfModel(model2,CUBE)
glueSetPos(model2,40.0,39.0,0.0)
glueSetScale(model2,1.0,40.0,40.0)
glueSetColider(model2,false)

local player = glueCreatePlayer()
glueSetGltfModel(player,gltfTEST)
glueSetScale(player,10.0,10.0,10.0)
glueSetColider(player,true)
glueSetColiderScale(player,1.0,1.0,0.3)
glueSetDefaultAnimationName(player,"Running")
glueBindCamera(player)
glueSetLuaPath(player,"LuaScripts/player.lua")

local pointLight = glueCreatePointLight()
glueSetLightColor(pointLight,1.0,1.0,1.0)
glueSetPos(pointLight,0.1,5.0,0.1)
glueBindObject(player,pointLight)

--[[
local pointLight2 = glueCreatePointLight()
glueSetLightColor(pointLight2,1.0,1.0,1.0)
glueSetPos(pointLight2,0.1,1.1,0.1)
glueBindObject(player,pointLight2)

local pointLight3 = glueCreatePointLight()
glueSetLightColor(pointLight3,1.0,1.0,1.0)
glueSetPos(pointLight3,-1.0,3.0,-1.0)
glueBindObject(player,pointLight3)

]]

local directionalLight = glueCreateDirectionalLight()
glueSetLightColor(directionalLight,0.5,0.5,0.5)
glueSetLightDirection(directionalLight,0.5,-1.0,0.0)
glueBindObject(player,directionalLight)

print("AAA")