#include"Scene.h"

Scene* Scene::instance = nullptr;

void Scene::init(std::string luaScriptPath)//luaファイルのパスを受け取る
{
	startPoint = glm::vec3(0.0f);

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
	player->setPosition(startPoint);//プレイヤーを初期位置に

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

	lua_pushnumber(lua, (int)GLTFOBJECT::ASPHALT);
	lua_setglobal(lua, "ASPHALT");//アスファルト

	lua_pushnumber(lua, (int)GLTFOBJECT::LEATHER);
	lua_setglobal(lua, "LEATHER");//白い革
}

void Scene::registerFunctions()//luaに関数を登録
{
	lua_register(lua, "glueCreateModel", glueSceneFunction::glueCreateModel);//3Dモデルクラスの追加
	lua_register(lua, "glueCreatePlayer", glueSceneFunction::glueCreatePlayer);//プレイヤーの追加
	lua_register(lua, "glueSetLuaPath", glueSceneFunction::glueSetLuaPath);//スクリプトをオブジェクトに追加
	lua_register(lua, "glueSetGltfModel", glueSceneFunction::glueSetGltfModel);//gltfモデルの追加
	lua_register(lua, "glueSetPos", glueSceneFunction::glueSetPos);//座標の設定
	lua_register(lua, "glueSetRotate", glueSceneFunction::glueSetRotate);//向きを設定
	lua_register(lua, "glueSetScale", glueSceneFunction::glueSetScale);//モデルのスケールの設定
	lua_register(lua, "glueSetBaseColor", glueSceneFunction::glueSetBaseColor);//Diffuseカラーの設定
	lua_register(lua, "glueBindCamera", glueSceneFunction::glueBindCamera);//カメラが追従するオブジェクトを設定する
	lua_register(lua, "glueSetColider", glueSceneFunction::glueSetColider);//モデルにコライダーを付ける
	lua_register(lua, "glueSetColiderScale", glueSceneFunction::glueSetColiderScale);//コライダーのスケールを設定
	lua_register(lua, "glueSetDefaultAnimationName", glueSceneFunction::glueSetDefaultAnimationName);//モデルにデフォルトのアニメーションを設定する
	lua_register(lua, "glueSetGravity", glueSceneFunction::glueSetGravity);//オブジェクトに重量を効かせる
	lua_register(lua, "glueSetSlippery", glueSceneFunction::glueSetSlippery);//摩擦力を設定する
	lua_register(lua, "glueCreatePointLight", glueSceneFunction::glueCreatePointLight);//ポイントライトを作成
	lua_register(lua, "glueSetLightColor", glueSceneFunction::glueSetLightColor);//ライトのカラーを設定
	lua_register(lua, "glueCreateDirectionalLight", glueSceneFunction::glueCreateDirectionalLight);//平行光源を作成
	lua_register(lua, "glueSetLightDirection", glueSceneFunction::glueSetLightDirection);//平行光源の方向を設定
	lua_register(lua, "glueBindObject", glueSceneFunction::glueBindObject);//オブジェクトに親子関係を設定する
	lua_register(lua, "glueSetStartPoint", glueSceneFunction::glueSetStartPoint);//初期座標の設定
	lua_register(lua, "glueSetLimitY", glueSceneFunction::glueSetLimitY);//y座標の下限の設定
}

void Scene::setStartPoint(glm::vec3 point)
{
	startPoint = point;
}

void Scene::setLimitY(float y)
{
	limitY = y;
}

void Scene::resetStatus()
{
	for (auto itr = sceneModels.begin(); itr != sceneModels.end(); itr++)
	{
		(*itr)->clearGroundingObject();//接地されているオブジェクトの登録の初期化
	}
}

bool Scene::UpdateScene()//シーン全体のアップデート処理
{
	bool exit = false;

	camera->Update();

	player->Update();
	player->updateTransformMatrix();

	for (int i = 0; i < sceneModels.size(); i++)
	{
		sceneModels[i]->Update();

		if (sceneModels[i]->uniformBufferChange)//オブジェクトの位置や向きが変わった場合
		{
			sceneModels[i]->updateTransformMatrix();//MVPのモデル行列を更新する
		}
	}

	//std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(100)));//処理を停止する

	resetStatus();//シーン全体のオブジェクトのリセット処理を行う

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
				if (sceneModels[i]->getColider()->Intersect(sceneModels[j]->getColider(), collisionVector))//GJKによる当たり判定を行う
				{
					if (sceneModels[i]->isMovable)//壁など動かさないものは除外する
					{
						sceneModels[i]->setPosition(sceneModels[i]->getPosition() + collisionVector);//衝突を解消する
						if (groundCollision(collisionVector))
						{
							sceneModels[i]->addGroundingObject(sceneModels[j]);
						}
					}

					if (sceneModels[j]->isMovable)
					{
						sceneModels[j]->setPosition(sceneModels[j]->getPosition() - collisionVector);
						if (groundCollision(-collisionVector))
						{
							sceneModels[j]->addGroundingObject(sceneModels[i]);
						}
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

		if (player->getColider()->Intersect(sceneModels[i]->getColider(), collisionVector))
		{
			if (sceneModels[i]->isMovable)
			{
				sceneModels[i]->setPosition(sceneModels[i]->getPosition() + collisionVector);
			}

			if (player->isMovable)
			{
				if (groundCollision(collisionVector))
				{
					player->cancelGravity();
					sceneModels[i]->addGroundingObject(player);
				}

				player->setPosition(player->getPosition() - collisionVector);
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

	if (player->getPosition().y < limitY)
	{
		player->restart(startPoint);
	}

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

bool Scene::groundCollision(glm::vec3 collisionVector)//滑り具合を調整する
{
	glm::vec3 v = -glm::normalize(collisionVector);

	float dot = glm::dot(v, up);

	return dot > 0;
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