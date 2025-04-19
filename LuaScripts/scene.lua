--Modelオブジェクトの作成
local model = glueCreateModel()
--Modelオブジェクトにgltfモデルをセット
glueSetGltfModel(model,"models/mountain_terrain.glb")
--初期座標の設定
glueSetPos(model,0.0,500.0,0.0)
--スケールの設定
glueSetScale(model,10.0,10.0,10.0)
--回転の設定
glueSetRotate(model,0.0,0.0,0.0)
--コライダーを付与する
--第二引数として、衝突解消時、このModelも動かすかどうかを決める
--このModelは床のため動かさない、よってfalse
--glueSetAABBColider(model,false)

local model2 = glueCreateModel()
glueSetGltfModel(model2,"models/leather.glb")
glueSetPos(model2,-250.0,0.0,0.0)
glueSetScale(model2,50.0,10.0,50.0)
glueSetAABBColider(model2,false)
--Modelの行動パターン用のluaスクリプトの設定
glueSetLuaPath(model2,"LuaScripts/wall1.lua")

local bridge1 = glueCreateModel()
glueSetGltfModel(bridge1,"models/leather.glb")
glueSetPos(bridge1,-700.0,0.0,0.0)
glueSetRotate(bridge1,0.0,0.0,0.0)
glueSetScale(bridge1,280.0,10.0,50.0)
glueSetAABBColider(bridge1,false)

local model3 = glueCreateModel()
glueSetGltfModel(model3,"models/leather.glb")
glueSetPos(model3,-600.0,60.0,-15.0)
glueSetScale(model3,50.0,50.0,50.0)
glueSetAABBColider(model3,false)
glueSetLuaPath(model3,"LuaScripts/wall2.lua")

local model4 = glueCreateModel()
glueSetGltfModel(model4,"models/leather.glb")
glueSetPos(model4,-700.0,60.0,-5.0)
glueSetScale(model4,50.0,50.0,50.0)
glueSetAABBColider(model4,false)
glueSetLuaPath(model4,"LuaScripts/wall2.lua")

local model5 = glueCreateModel()
glueSetGltfModel(model5,"models/leather.glb")
glueSetPos(model5,-800.0,60.0,5.0)
glueSetScale(model5,50.0,50.0,50.0)
glueSetAABBColider(model5,false)
glueSetLuaPath(model5,"LuaScripts/wall2.lua")

local spineFloor1 = glueCreateModel()
glueSetGltfModel(spineFloor1,"models/wood.glb")
glueSetPos(spineFloor1,-1050.0,0.0,20.0)
glueSetScale(spineFloor1,50.0,10.0,50.0)
glueSetAABBColider(spineFloor1,false)
glueSetLuaPath(spineFloor1,"LuaScripts/rotate_floor.lua")

local spineFloor2 = glueCreateModel()
glueSetGltfModel(spineFloor2,"models/wood.glb")
glueSetPos(spineFloor2,-1225.0,0.0,20.0)
glueSetScale(spineFloor2,50.0,10.0,50.0)
glueSetAABBColider(spineFloor2,false)
glueSetLuaPath(spineFloor2,"LuaScripts/rotate_floor.lua")
glueSetDelayStartLua(spineFloor2,240)

local spineFloor3 = glueCreateModel()
glueSetGltfModel(spineFloor3,"models/wood.glb")
glueSetPos(spineFloor3,-1375.0,0.0,20.0)
glueSetScale(spineFloor3,50.0,10.0,50.0)
glueSetAABBColider(spineFloor3,false)
glueSetLuaPath(spineFloor3,"LuaScripts/rotate_floor.lua")
glueSetDelayStartLua(spineFloor3,480)

--プレイヤーの作成
local player = glueCreatePlayer()
glueSetGltfModel(player,"models/PlayerModel.glb")
glueSetScale(player,10.0,10.0,10.0)
glueSetAABBColider(player,true)
glueSetColiderScale(player,1.0,1.0,0.3)
--アイドル時に再生するアニメーションを指定
glueSetDefaultAnimationName(player,"Idle")
--このオブジェクトをカメラに追従させる
glueBindCamera(player)
glueSetLuaPath(player,"LuaScripts/player.lua")

--ポイントライトの作成
local pointLight = glueCreatePointLight()
--光の色の設定
glueSetLightColor(pointLight,0.1,0.1,0.1)
glueSetPos(pointLight,1.1,5.0,1.1)
--プレイヤーを親として追従するようにする
glueBindObject(player,pointLight)

local pointLight2 = glueCreatePointLight()
glueSetLightColor(pointLight2,0.1,0.1,0.1)
glueSetPos(pointLight2,1.1,1.1,3.1)
glueBindObject(player,pointLight2)

local pointLight3 = glueCreatePointLight()
glueSetLightColor(pointLight3,0.1,0.1,0.1)
glueSetPos(pointLight3,-1.0,3.0,-1.0)
glueBindObject(player,pointLight3)

--平行光源の作成
local directionalLight = glueCreateDirectionalLight()
glueSetLightColor(directionalLight,0.5,0.5,0.5)
--シャドウマップ作成用にかなり遠くに設定
glueSetLightDirection(directionalLight,-200.0,-200.0,-200.0)
--プレイヤーを追従する
glueBindObject(player,directionalLight)
glueSetPos(directionalLight,200,200,200)

--ステージの下限を設定
glueSetLimitY(-300)
--リスタート時の座標を設定
glueSetStartPoint(0.0,100.0,0.0)

--背景やIBLで使うHDRI画像の設定
glueSetHDRIMap("textures/grass.hdr")

print("AAA")