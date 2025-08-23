--Modelオブジェクトの作成
local model = createModel()
--Modelオブジェクトにgltfモデルをセット
setGltfModel(model,"models/Ground.glb")
--初期座標の設定
setPos(model,0.0,0.0,0.0)
--スケールの設定
setScale(model,3000.0,3000.0,3000.0)
--回転の設定
setRotate(model,0.0,0.0,0.0)
--コライダーを付与する
--第二引数として、衝突解消時、このModelも動かすかどうかを決める
--このModelは床のため動かさない、よってfalse
setAABBColider(model,false,false)
setUVScale(model)


--[[local model2 = createModel()
--Modelオブジェクトにgltfモデルをセット
setGltfModel(model2,"models/robot.glb")
--初期座標の設定
setPos(model2,0.0,20.0,0.0)
--スケールの設定
setScale(model2,30.0,30.0,30.0)
--回転の設定
--setRotate(model2,0.0,0.0,0.0)
--コライダーを付与する
--第二引数として、衝突解消時、このModelも動かすかどうかを決める
--このModelは床のため動かさない、よってfalse
setAABBColider(model2,false,false)
setDefaultAnimationName(model2,"run")
setColiderScale(model2,1.0,1.0,0.3)
]]

--プレイヤーの作成
local player = createPlayer()
setGltfModel(player,"models/robot.glb")
setScale(player,30.0,30.0,30.0)
setAABBColider(player,true,false)
setColiderScale(player,1.0,1.0,0.3)
--アイドル時に再生するアニメーションを指定
setDefaultAnimationName(player,"Idle_gunMiddle")
--このオブジェクトをカメラに追従させる
bindCamera(player)
setLuaPath(player,"LuaScripts/player.lua")
setTransparent(player,true)

--ポイントライトの作成
local pointLight = createPointLight()
--光の色の設定
setLightColor(pointLight,0.1,0.1,0.1)
setPos(pointLight,1.1,5.0,1.1)
--プレイヤーを親として追従するようにする
bindObject(player,pointLight)

local pointLight2 = createPointLight()
setLightColor(pointLight2,0.1,0.1,0.1)
setPos(pointLight2,1.1,1.1,3.1)
bindObject(player,pointLight2)

local pointLight3 = createPointLight()
setLightColor(pointLight3,0.1,0.1,0.1)
setPos(pointLight3,-1.0,3.0,-1.0)
bindObject(player,pointLight3)

--平行光源の作成
local directionalLight = createDirectionalLight()
setLightColor(directionalLight,0.5,0.5,0.5)
--シャドウマップ作成用にかなり遠くに設定
setLightDirection(directionalLight,-200.0,-200.0,-200.0)
--プレイヤーを追従する
bindObject(player,directionalLight)
setPos(directionalLight,200,200,200)

--ステージの下限を設定
setLimitY(-300)
--リスタート時の座標を設定
setStartPoint(0.0,100.0,0.0)

--背景やIBLで使うHDRI画像の設定
setHDRIMap("textures/grass.hdr")

--ゲーム内で使うリソースを読み込んでおく
loadGltfModel("models/beamBullet.glb")

createText("あいうえお\nあああ")

print("lua finish")