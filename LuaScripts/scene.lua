--Modelオブジェクトの作成
local model = glueCreateModel()
--Modelオブジェクトにgltfモデルをセット
glueSetGltfModel(model,"models/concrete.glb")
--初期座標の設定
glueSetPos(model,0.0,0.0,0.0)
--スケールの設定
glueSetScale(model,100.0,10.0,100.0)
--回転の設定
glueSetRotate(model,0.0,0.0,0.0)
--コライダーを付与する
--第二引数として、衝突解消時、このModelも動かすかどうかを決める
--このModelは床のため動かさない、よってfalse
glueSetAABBColider(model,false)
glueSetUVScale(model)

--プレイヤーの作成
local player = glueCreatePlayer()
glueSetGltfModel(player,"models/robot.glb")
glueSetScale(player,30.0,30.0,30.0)
glueSetAABBColider(player,true)
glueSetColiderScale(player,1.0,1.0,0.3)
--アイドル時に再生するアニメーションを指定
glueSetDefaultAnimationName(player,"Idle_gunMiddle")
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