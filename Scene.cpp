#include"Scene.h"

Scene* Scene::instance = nullptr;

void Scene::init(std::string luaScriptPath)//luaファイルのパスを受け取る
{

	camera = std::make_shared<Camera>();

	initLuaScript(luaScriptPath);//luaからステージのデータを読み取り

	initFrameSetting();//luaから読み取ったオブジェクトの初期化処理
}

Scene::Scene()
{
}

Scene::~Scene()
{
}

void Scene::initFrameSetting()
{
	for (int i = 0; i < sceneModels.size(); i++)//すべてのオブジェクトの初期化処理
	{
		sceneModels[i]->initFrameSetting();//オブジェクトの初期化処理
	}
	player->initFrameSetting();//プレイヤークラスのみ別枠

	setLights();//ライトの他のクラスのデータを用意
	setModels();//モデルの他のクラスのデータを用意
}

void Scene::initLuaScript(std::string path)//luaスクリプトの読み取り
{
	lua = luaL_newstate();
	luaL_openlibs(lua);

	lua_pushlightuserdata(lua, this);
	lua_setglobal(lua, "Scene");//インスタンスをluaスクリプトで使えるようにする

	registerOBJECT();
	registerFunctions();

	luaL_dofile(lua,path.c_str());
}

void Scene::registerOBJECT()//enumをスクリプトと共有する
{
	lua_pushnumber(lua, (int)GLTFOBJECT::gltfTEST);
	lua_setglobal(lua, "gltfTEST");//キャラクター

	lua_pushnumber(lua, (int)GLTFOBJECT::CUBE);
	lua_setglobal(lua, "CUBE");//立方体
}

void Scene::registerFunctions()//luaに関数を登録
{
	lua_register(lua, "glueCreateModel", glueCreateModel);//3Dモデルクラスの追加
	lua_register(lua, "glueCreatePlayer", glueCreatePlayer);//プレイヤーの追加
	lua_register(lua, "glueSetLuaPath", glueSetLuaPath);//スクリプトをオブジェクトに追加
	lua_register(lua, "glueSetGltfModel",glueSetGltfModel);//gltfモデルの追加
	lua_register(lua, "glueSetPos", glueSetPos);//座標の設定
	lua_register(lua, "glueSetRotate", glueSetRotate);//向きを設定
	lua_register(lua, "glueSetScale", glueSetScale);//モデルのスケールの設定
	lua_register(lua, "glueSetDiffuse", glueSetDiffuse);//Diffuseカラーの設定
	lua_register(lua, "glueBindCamera", glueBindCamera);//カメラが追従するオブジェクトを設定する
	lua_register(lua, "glueSetColider", glueSetColider);//モデルにコライダーを付ける
	lua_register(lua, "glueSetColiderScale", glueSetColiderScale);//コライダーのスケールを設定
	lua_register(lua, "glueSetDefaultAnimationName", glueSetDefaultAnimationName);//モデルにデフォルトのアニメーションを設定する
	lua_register(lua, "glueSetGravity", glueSetGravity);//オブジェクトに重量を効かせる
	lua_register(lua, "glueSetSlippery", glueSetSlippery);//摩擦力を設定する
	lua_register(lua, "glueCreatePointLight", glueCreatePointLight);//ポイントライトを作成
	lua_register(lua, "glueSetLightColor", glueSetLightColor);//ライトのカラーを設定
	lua_register(lua, "glueCreateDirectionalLight", glueCreateDirectionalLight);//平行光源を作成
	lua_register(lua, "glueSetLightDirection", glueSetLightDirection);//平行光源の方向を設定
	lua_register(lua, "glueBindObject", glueBindObject);//オブジェクトに親子関係を設定する
}

bool Scene::UpdateScene()//シーン全体のアップデート処理
{
	bool exit = false;

	camera->Update();

	player->Update();

	for (int i = 0; i < sceneModels.size(); i++)
	{
		sceneModels[i]->Update();

		if (sceneModels[i]->uniformBufferChange)//オブジェクトの位置や向きが変わった場合
		{
			sceneModels[i]->updateTransformMatrix();//MVPのモデル行列を更新する
		}
	}

	for (int i = 0; i < sceneModels.size() - 1; i++)//モデル同士の当たり判定を行う
	{
		if (!sceneModels[i]->hasColider())//コライダーを持っていなかったらスキップ
		{
			continue;
		}

		for (int j = i + 1; j < sceneModels.size(); j++)
		{
			if (sceneModels[j]->hasColider())
			{
				if (sceneModels[i]->getColider()->Intersect(sceneModels[j]->getColider(), collisionDepth, collisionVector))//GJKによる当たり判定を行う
				{
					if (sceneModels[i]->isMovable)//壁など動かさないものは除外する
					{
						sceneModels[i]->setPosition(sceneModels[i]->getPosition() + slopeCollision(collisionVector));//衝突を解消する
					}

					if (sceneModels[j]->isMovable)
					{
						sceneModels[j]->setPosition(sceneModels[j]->getPosition() + slopeCollision(collisionVector));
					}
				}
			}
		}
	}

	//Playerクラスとモデルクラスの当たり判定
	for (int i = 0; i < sceneModels.size(); i++)
	{
		if (!sceneModels[i]->hasColider())
		{
			continue;
		}

		if (player->getColider()->Intersect(sceneModels[i]->getColider(), collisionDepth, collisionVector))
		{
			if (sceneModels[i]->isMovable)
			{
				sceneModels[i]->setPosition(sceneModels[i]->getPosition() + slopeCollision(collisionVector));
			}

			if (player->isMovable)
			{
				player->setPosition(player->getPosition() + slopeCollision(-collisionVector));
			}
		}
	}

	for (int i = 0; i < sceneModels.size(); i++)
	{
		if (sceneModels[i]->uniformBufferChange)//衝突解消によってモデルが移動したら、再度行列の更新
		{
			sceneModels[i]->updateTransformMatrix();
		}
	}
	player->updateTransformMatrix();

	return exit;
}

void Scene::setModels()
{
	Storage::GetInstance()->setCamera(camera);

	//描画するモデルのポインタを積んでいく
	for (int i = 0;i < sceneModels.size();i++)
	{
		sceneModels[i]->updateTransformMatrix();
		Storage::GetInstance()->addModel(sceneModels[i]);
	}

	player->updateTransformMatrix();
	Storage::GetInstance()->addModel(player);
}

void Scene::setLights()
{
	for (std::shared_ptr<PointLight> pl : scenePointLights)
	{
		pl->updateTransformMatrix();
		Storage::GetInstance()->addLight(pl);
	}

	for (std::shared_ptr<DirectionalLight> dl : sceneDirectionalLights)
	{
		dl->updateTransformMatrix();
		Storage::GetInstance()->addLight(dl);
	}

	//LightにVulkanでの変数などを持たせる
	Storage::GetInstance()->prepareLightsForVulkan();
}

glm::vec3 Scene::slopeCollision(glm::vec3 collisionVector)//滑り具合を調整する
{
	if (collisionVector.y > 0.5f)//侵入しているベクトルが上向きだったら
	{
		return collisionVector * glm::vec3(0.0f, 1.0f, 0.0f);//床が斜めでも上向きに移動するようにする
															 //斜め上に移動させると、無限に斜め下に落ちて行ってしまう
	}
	else
	{
		return collisionVector;
	}
}

std::shared_ptr<Model> Scene::raycast(glm::vec3 origin, glm::vec3 dir, float length,Model* model)//光線を飛ばして、コライダーを探す
{
	for (int i = 0; i < sceneModels.size(); i++)
	{
		if (model == sceneModels[i].get())
		{
			continue;
		}

		if (!sceneModels[i]->hasColider())
		{
			continue;
		}

		std::shared_ptr<Colider> colider2 = sceneModels[i]->getColider();
		if (colider2->Intersect(origin,dir,length))//コライダーと線分の衝突判定
		{
			return sceneModels[i];
		}
	}

	return nullptr;
}