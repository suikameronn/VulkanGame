#include"Scene.h"

Scene* Scene::instance = nullptr;

void Scene::init(std::string luaScriptPath)//luaファイルのパスを受け取る
{
	startPoint = glm::vec3(0.0f);

	camera = std::make_shared<Camera>();
	Storage::GetInstance()->setCamera(camera);

	initLuaScript(luaScriptPath);//luaからステージのデータを読み取り

	initFrameSetting();//luaから読み取ったオブジェクトの初期化処理
}

Scene::Scene()
{
}

Scene::~Scene()
{
}

//初回フレームのみ実行 ステージ上のすべてのオブジェクトの初回フレーム時の設定を行う
void Scene::initFrameSetting()
{
	for (int i = 0; i < sceneModels.size(); i++)//すべてのオブジェクトの初期化処理
	{
		sceneModels[i]->initFrameSetting();//オブジェクトの初期化処理
	}
	player->initFrameSetting();//プレイヤークラスのみ別枠
	player->setPosition(startPoint);//プレイヤーを初期位置に

	Storage::GetInstance()->prepareDescriptorSets();

	setLights();//ライトの他のクラスのデータを用意

	Storage::GetInstance()->prepareLightsForVulkan();//LightにVulkanでの変数などを持たせる
	Storage::GetInstance()->prepareDescriptorData();//descriptorSetの用意

	setModels();//モデルの他のクラスのデータを用意
}

//luaスクリプトの読み取り、実行
void Scene::initLuaScript(std::string path)
{
	lua = luaL_newstate();
	luaL_openlibs(lua);

	lua_pushlightuserdata(lua, this);//このSceneのインスタンスをluaの仮想マシンに送る
	lua_setglobal(lua, "Scene");//インスタンスをluaスクリプトで使えるようにする

	registerOBJECT();//gltfモデルを指定しるための番号を送る
	registerFunctions();//スクリプトから呼び出す関数を登録する

	luaL_dofile(lua,path.c_str());//スクリプトの実行
}

//luaの仮想マシンにgltfモデルを指定するための番号を設定する
void Scene::registerOBJECT()//enumをスクリプトと共有する
{
	lua_pushnumber(lua, (int)GLTFOBJECT::gltfTEST);
	lua_setglobal(lua, "gltfTEST");//キャラクター

	lua_pushnumber(lua, (int)GLTFOBJECT::ASPHALT);
	lua_setglobal(lua, "ASPHALT");//アスファルト

	lua_pushnumber(lua, (int)GLTFOBJECT::LEATHER);
	lua_setglobal(lua, "LEATHER");//白い革
}

void Scene::registerFunctions()//luaから呼び出される静的関数を設定
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
	lua_register(lua, "glueSetAABBColider", glueSceneFunction::glueSetAABBColider);//モデルにAABBコライダーを付ける
	lua_register(lua, "glueSetConvexColider", glueSceneFunction::glueSetConvexColider);//モデルに凸包のコライダーを付ける
	lua_register(lua, "glueSetColiderScale", glueSceneFunction::glueSetColiderScale);//コライダーのスケールを設定
	lua_register(lua, "glueSetDefaultAnimationName", glueSceneFunction::glueSetDefaultAnimationName);//モデルにデフォルトのアニメーションを設定する
	lua_register(lua, "glueSetGravity", glueSceneFunction::glueSetGravity);//オブジェクトに重量を効かせる
	lua_register(lua, "glueCreatePointLight", glueSceneFunction::glueCreatePointLight);//ポイントライトを作成
	lua_register(lua, "glueSetLightColor", glueSceneFunction::glueSetLightColor);//ライトのカラーを設定
	lua_register(lua, "glueCreateDirectionalLight", glueSceneFunction::glueCreateDirectionalLight);//平行光源を作成
	lua_register(lua, "glueSetLightDirection", glueSceneFunction::glueSetLightDirection);//平行光源の方向を設定
	lua_register(lua, "glueBindObject", glueSceneFunction::glueBindObject);//オブジェクトに親子関係を設定する
	lua_register(lua, "glueSetStartPoint", glueSceneFunction::glueSetStartPoint);//初期座標の設定
	lua_register(lua, "glueSetLimitY", glueSceneFunction::glueSetLimitY);//y座標の下限の設定
	lua_register(lua, "glueSetHDRIMap", glueSceneFunction::glueSetHDRIMap);//HDRI画像の設定
}

//初期座標の設定
void Scene::setStartPoint(glm::vec3 point)
{
	startPoint = point;
}

//y座標の下限の設定
void Scene::setLimitY(float y)
{
	limitY = y;
}

//HDRIマップの設定
void Scene::setHDRIMap(std::string imagePath)
{
	hdriMap = FileManager::GetInstance()->loadImage(imagePath);
	Storage::GetInstance()->setCubemapTexture(hdriMap);
}

//オブジェクトの接地判定などをリセット
void Scene::resetStatus()
{
	for (auto itr = sceneModels.begin(); itr != sceneModels.end(); itr++)
	{
		(*itr)->clearGroundingObject();//接地されているオブジェクトの登録の初期化
		(*itr)->isGrounding = false;
	}

	player->isGrounding = false;
}

bool Scene::UpdateScene()//ステージ上のオブジェクトなどの更新処理
{
	bool exit = false;

	camera->Update();//カメラの更新処理

	player->Update();//プレイヤーの更新処理
	player->updateTransformMatrix();//プレイヤーの座標変換行列の更新

	for (int i = 0; i < sceneModels.size(); i++)//すべてのオブジェクトの更新処理
	{
		sceneModels[i]->Update();//オブジェクトの更新処理

		if (sceneModels[i]->uniformBufferChange)//オブジェクトの位置や向きが変わった場合
		{
			sceneModels[i]->updateTransformMatrix();//MVPのモデル行列を更新する
		}
	}

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
						if (groundCollision(collisionVector))//もし一つ目のオブジェクトが二つめのオブジェクトの真上にある
															 //つまり、床のようなオブジェクトの上に載っている状況の場合
						{
							sceneModels[j]->isGrounding = true;
							sceneModels[i]->addGroundingObject(sceneModels[j]);//床のオブジェクトの移動に、上に載っているオブジェクトを追従させる
						}
					}

					if (sceneModels[j]->isMovable)//上と同様
					{
						sceneModels[j]->setPosition(sceneModels[j]->getPosition() - collisionVector);
						if (groundCollision(-collisionVector))
						{
							sceneModels[i]->isGrounding = true;
							sceneModels[j]->addGroundingObject(sceneModels[i]);
						}
					}
				}
			}
		}
	}

	//Playerクラスとモデルクラスの当たり判定
	//他、上のModelクラス同士の処理と同様
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
					player->isGrounding = true;
					player->cancelGravity();
					sceneModels[i]->addGroundingObject(player);
				}

				player->setPosition(player->getPosition() - collisionVector);
			}
		}
	}

	//当たり判定の処理により、移動したオブジェクトの座標変換行列やコライダーの位置を更新する
	for (int i = 0; i < sceneModels.size(); i++)
	{
		if (sceneModels[i]->uniformBufferChange)//衝突解消によってモデルが移動したら、再度行列の更新
		{
			sceneModels[i]->updateTransformMatrix();
		}
	}
	player->updateTransformMatrix();

	//もしプレイヤーがステージの下限を超えたら、リスタートさせる
	if (player->getPosition().y < limitY)
	{
		player->restart(startPoint);
	}

	return exit;//ウィンドウを閉じようとした場合は、falsを送り、ゲームの終了処理をさせる 
}

//ステージ上のオブジェクトにVulkanの変数を設定する
void Scene::setModels()
{
	//描画するモデルのポインタを積んでいく
	for (int i = 0;i < sceneModels.size();i++)
	{
		sceneModels[i]->updateTransformMatrix();//座標変換行列の更新
		Storage::GetInstance()->addModel(sceneModels[i]);//gpu上に頂点用などのバッファーを確保する
	}

	player->updateTransformMatrix();//座標変換行列の更新
	Storage::GetInstance()->addModel(player);//プレイヤーの扱う3Dモデルにおいても同様バッファーを確保する
}

//ステージ上のライトにVulkanの変数を設定する
void Scene::setLights()
{
	Storage* storage = Storage::GetInstance();

	std::shared_ptr<Model> cubemap = std::shared_ptr<Model>(new Model());//キューブマップ用の立方体の準備
	cubemap->setgltfModel(FileManager::GetInstance()->loadModel(GLTFOBJECT::CUBEMAP));;
	storage->setCubeMapModel(cubemap);

	for (std::shared_ptr<PointLight> pl : scenePointLights)
	{
		pl->updateTransformMatrix();
		storage->addLight(pl);//バッファーの確保
	}

	for (std::shared_ptr<DirectionalLight> dl : sceneDirectionalLights)
	{
		dl->updateTransformMatrix();
		storage->addLight(dl);//バッファーの確保
	}
}

//コライダーが床に接地しているかの判定 trueの場合はその床の移動を追従する
bool Scene::groundCollision(glm::vec3 collisionVector)
{
	glm::vec3 v = -glm::normalize(collisionVector);

	float dot = glm::dot(v, up);

	return dot >= 0;
}

//四角形を延ばして、衝突判定を行う、接地判定に使われる
std::shared_ptr<Model> Scene::raycast(glm::vec3 origin, glm::vec3 dir, float length,Model* model)
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