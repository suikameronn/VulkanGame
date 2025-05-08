#include"GameManager.h"

#include"Scene.h"

Scene* Scene::instance = nullptr;

void Scene::init(std::string luaScriptPath)//luaファイルのパスを受け取る
{
	startPoint = glm::vec3(0.0f);

	GameManager* manager = GameManager::GetInstance();

	camera = std::shared_ptr<Camera>(new Camera(manager->getWindowWidth(), manager->getWindowHeight()));
	Storage::GetInstance()->setCamera(camera);

	rtree = std::make_unique<RTree<Model>>();

	cubemap = std::shared_ptr<Cubemap>(new Cubemap());

	initLuaScript(luaScriptPath);//luaからステージのデータを読み取り

	initFrameSetting();//luaから読み取ったオブジェクトの初期化処理
}

Scene::Scene()
{
}

Scene::~Scene()
{
	cleanupVulkan();
}

//初回フレームのみ実行 ステージ上のすべてのオブジェクトの初回フレーム時の設定を行う
void Scene::initFrameSetting()
{
	Storage* storage = Storage::GetInstance();

	FileManager::GetInstance()->addLoadModelList("models/cubemap.glb", cubemap.get());

	//ModelクラスにgltfModelクラスを設定する
	FileManager::GetInstance()->setGltfModel();

	VulkanBase* vulkan = VulkanBase::GetInstance();

	for (int i = 0; i < sceneModels.size(); i++)//すべてのオブジェクトの初期化処理
	{
		vulkan->setModelData(sceneModels[i]);
		sceneModels[i]->initFrameSetting();//オブジェクトの初期化処理
	}

	vulkan->setModelData(player);
	player->initFrameSetting();//プレイヤークラスのみ別枠
	player->setPosition(startPoint);//プレイヤーを初期位置に

	vulkan->createCubemap(cubemap);

	for (int i = 0; i < sceneUI.size(); i++)
	{
		vulkan->setUI(sceneUI[i]);
		sceneUI[i]->initFrameSettings();
	}

	//シャドウマッピングの用意
	vulkan->prepareShadowMapping(static_cast<int>(sceneDirectionalLights.size()), shadowMapData);

	//ライトのバッファの用意
	vulkan->createPointLightBuffer(pointLightBuffer);
	vulkan->createDirectionalLightBuffer(dirLightBuffer);
}

//luaスクリプトの読み取り、実行
void Scene::initLuaScript(std::string path)
{
	lua = luaL_newstate();
	luaL_openlibs(lua);

	lua_pushlightuserdata(lua, this);//このSceneのインスタンスをluaの仮想マシンに送る
	lua_setglobal(lua, "Scene");//インスタンスをluaスクリプトで使えるようにする

	registerFunctions();//スクリプトから呼び出す関数を登録する

	luaL_dofile(lua,path.c_str());//スクリプトの実行
}

void Scene::registerFunctions()//luaから呼び出される静的関数を設定
{
	lua_register(lua, "createModel", glueSceneFunction::glueCreateModel);//3Dモデルクラスの追加
	lua_register(lua, "createPlayer", glueSceneFunction::glueCreatePlayer);//プレイヤーの追加
	lua_register(lua, "setLuaPath", glueSceneFunction::glueSetLuaPath);//スクリプトをオブジェクトに追加
	lua_register(lua, "setDelayStartLua", glueSceneFunction::glueSetDelayStartLua);//スクリプトの実行開始タイミングを設定
	lua_register(lua, "setGltfModel", glueSceneFunction::glueSetGltfModel);//gltfモデルの追加
	lua_register(lua, "setPos", glueSceneFunction::glueSetPos);//座標の設定
	lua_register(lua, "setRotate", glueSceneFunction::glueSetRotate);//向きを設定
	lua_register(lua, "setScale", glueSceneFunction::glueSetScale);//モデルのスケールの設定
	lua_register(lua, "setBaseColor", glueSceneFunction::glueSetBaseColor);//Diffuseカラーの設定
	lua_register(lua, "bindCamera", glueSceneFunction::glueBindCamera);//カメラが追従するオブジェクトを設定する
	lua_register(lua, "setAABBColider", glueSceneFunction::glueSetAABBColider);//モデルにAABBコライダーを付ける
	lua_register(lua, "setColiderScale", glueSceneFunction::glueSetColiderScale);//コライダーのスケールを設定
	lua_register(lua, "setDefaultAnimationName", glueSceneFunction::glueSetDefaultAnimationName);//モデルにデフォルトのアニメーションを設定する
	lua_register(lua, "setGravity", glueSceneFunction::glueSetGravity);//オブジェクトに重量を効かせる
	lua_register(lua, "createPointLight", glueSceneFunction::glueCreatePointLight);//ポイントライトを作成
	lua_register(lua, "setLightColor", glueSceneFunction::glueSetLightColor);//ライトのカラーを設定
	lua_register(lua, "createDirectionalLight", glueSceneFunction::glueCreateDirectionalLight);//平行光源を作成
	lua_register(lua, "setLightDirection", glueSceneFunction::glueSetLightDirection);//平行光源の方向を設定
	lua_register(lua, "bindObject", glueSceneFunction::glueBindObject);//オブジェクトに親子関係を設定する
	lua_register(lua, "setStartPoint", glueSceneFunction::glueSetStartPoint);//初期座標の設定
	lua_register(lua, "setLimitY", glueSceneFunction::glueSetLimitY);//y座標の下限の設定
	lua_register(lua, "setHDRIMap", glueSceneFunction::glueSetHDRIMap);//HDRI画像の設定
	lua_register(lua, "setUVScale", glueSceneFunction::glueSetUVScale);//uvを調整するようにして、テクスチャの引き延ばしを防ぐ
	lua_register(lua, "setTransparent", glueSceneFunction::glueSetTransparent);//半透明の表示のフラッグを設定
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
	cubemap->setHDRIMap(FileManager::GetInstance()->loadImage(imagePath));
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

int Scene::UpdateScene()//ステージ上のオブジェクトなどの更新処理
{
	int exit = GAME_CONTINUE;

	camera->Update();//カメラの更新処理
	camera->updateTransformMatrix();

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

	//当たり判定
	rtreeIntersect();
	//intersect();

	//もしプレイヤーがステージの下限を超えたら、リスタートさせる
	if (player->getPosition().y < limitY)
	{
		player->restart(startPoint);
	}

	//フレーム終了時に、バッファの更新などをする
	frameEnd();

	if (Controller::GetInstance()->getKey(GLFW_KEY_ESCAPE))
	{
		exit = GAME_FINISH;
	}

	render();

	return exit;//ウィンドウを閉じようとした場合は、falsを送り、ゲームの終了処理をさせる 
}

//コライダーが床に接地しているかの判定 trueの場合はその床の移動を追従する
bool Scene::groundCollision(glm::vec3 collisionVector)
{
	glm::vec3 v = -glm::normalize(collisionVector);

	float dot = glm::dot(v, up);

	return dot >= 0;
}

//四角形を延ばして、衝突判定を行う、接地判定に使われる
std::shared_ptr<Model> Scene::raycast(glm::vec3 origin, glm::vec3 dir, float length,Model* model,glm::vec3& normal)
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
		if (colider2->Intersect(origin,dir,length,normal))//コライダーと線分の衝突判定
		{
			return sceneModels[i];
		}
	}

	return nullptr;
}

//シーン全体のR木にオブジェクトを追加する
void Scene::addModelToRTree(Model* model)
{
	rtree->insert(model, model->getMbrMin(), model->getMbrMax());
}

//ツリー内のオブジェクトの位置を更新する
void Scene::updateObjectPos(Model* model, RNode<Model>* node)
{
	rtree->reflectMove(model, node);
}

void Scene::rtreeIntersect()
{
	for (int i = 0; i < sceneModels.size(); i++)//モデル同士の当たり判定を行う
	{
		if (!sceneModels[i]->hasColider())//コライダーを持っていなかったらスキップ
		{
			continue;
		}

		//R木を使って、当たり判定を行うオブジェクトを絞る
		std::vector<Model*> collisionDetectTarget;
		rtree->broadPhaseCollisionDetection(collisionDetectTarget, sceneModels[i]->getMbrMin(), sceneModels[i]->getMbrMax());

		for (int j = 0; j < collisionDetectTarget.size(); j++)
		{
			//コライダーが設定されているかつ、自分自身を除くオブジェクトと当たり判定を行う
			if (collisionDetectTarget[j]->hasColider() && sceneModels[i].get() == collisionDetectTarget[j])
			{
				if (sceneModels[i]->getColider()->Intersect(collisionDetectTarget[j]->getColider(), collisionVector))//GJKによる当たり判定を行う
				{
					if (sceneModels[i]->isMovable)//壁など動かさないものは除外する
					{
						sceneModels[i]->setPosition(sceneModels[i]->getPosition() + collisionVector);//衝突を解消する
						if (groundCollision(collisionVector))//もし一つ目のオブジェクトが二つめのオブジェクトの真上にある
							//つまり、床のようなオブジェクトの上に載っている状況の場合
						{
							collisionDetectTarget[j]->isGrounding = true;
							sceneModels[i]->addGroundingObject(collisionDetectTarget[j]);//床のオブジェクトの移動に、上に載っているオブジェクトを追従させる
						}
					}

					if (collisionDetectTarget[j]->isMovable)//上と同様
					{
						collisionDetectTarget[j]->setPosition(collisionDetectTarget[j]->getPosition() - collisionVector);
						if (groundCollision(-collisionVector))
						{
							sceneModels[i]->isGrounding = true;
							collisionDetectTarget[j]->addGroundingObject(sceneModels[i].get());
						}
					}
				}
			}
		}
	}

	{//プレイヤーキャラクターについての当たり判定を行う
		std::vector<Model*> collisionDetectTarget;
		rtree->broadPhaseCollisionDetection(collisionDetectTarget, player->getMbrMin(), player->getMbrMax());

		for (int i = 0; i < collisionDetectTarget.size(); i++)
		{
			if (collisionDetectTarget[i]->getColider())
			{
				if (player->getColider()->Intersect(collisionDetectTarget[i]->getColider(), collisionVector))
				{
					if (collisionDetectTarget[i]->isMovable)
					{
						collisionDetectTarget[i]->setPosition(collisionDetectTarget[i]->getPosition() + collisionVector);
					}

					if (player->isMovable)
					{
						if (groundCollision(collisionVector))
						{
							player->isGrounding = true;
							collisionDetectTarget[i]->addGroundingObject(player.get());
						}

						player->setPosition(player->getPosition() - collisionVector);
					}
				}
			}
		}
	}
}

void Scene::intersect()
{
	for (int i = 0; i < sceneModels.size() - 1; i++)//モデル同士の当たり判定を行う
	{
		if (!sceneModels[i]->hasColider())//コライダーを持っていなかったらスキップ
		{
			continue;
		}

		for (int j = i + 1; j < sceneModels.size(); j++)
		{
			if (!sceneModels[j]->hasColider())
			{
				continue;
			}

			//コライダーが設定されているかつ、自分自身を除くオブジェクトと当たり判定を行う
			if (sceneModels[i]->getColider()->Intersect(sceneModels[j]->getColider(), collisionVector))//GJKによる当たり判定を行う
			{
				if (sceneModels[i]->isMovable)//壁など動かさないものは除外する
				{
					sceneModels[i]->setPosition(sceneModels[i]->getPosition() + collisionVector);//衝突を解消する
					if (groundCollision(collisionVector))//もし一つ目のオブジェクトが二つめのオブジェクトの真上にある
						//つまり、床のようなオブジェクトの上に載っている状況の場合
					{
						sceneModels[j]->isGrounding = true;
						sceneModels[i]->addGroundingObject(sceneModels[j].get());//床のオブジェクトの移動に、上に載っているオブジェクトを追従させる
					}
				}

				if (sceneModels[j]->isMovable)//上と同様
				{
					sceneModels[j]->setPosition(sceneModels[j]->getPosition() - collisionVector);
					if (groundCollision(-collisionVector))
					{
						sceneModels[i]->isGrounding = true;
						sceneModels[j]->addGroundingObject(sceneModels[i].get());
					}
				}
			}
		}
	}

	//プレイヤーキャラクターについての当たり判定を行う
	for (int i = 0; i < sceneModels.size(); i++)
	{
		if (sceneModels[i]->getColider())
		{
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
						sceneModels[i]->addGroundingObject(player.get());
					}

					player->setPosition(player->getPosition() - collisionVector);
				}
			}
		}
	}
}

//フレーム終了時に実行
void Scene::frameEnd()
{
	//ライトのユニフォームバッファの更新は別枠で行う
	updateLightUniformBuffer();

	//当たり判定の処理により、移動したオブジェクトの座標変換行列やコライダーの位置を更新する
	//ユニフォームバッファの更新
	for (auto model : sceneModels)
	{
		model->frameEnd(sceneDirectionalLights, scenePointLights, shadowMapData);
	}
	player->frameEnd(sceneDirectionalLights,scenePointLights,shadowMapData);
	cubemap->frameEnd();

	for (auto ui : sceneUI)
	{
		ui->frameEnd();
	}
}

//ライトとシャドウマップのユニフォームバッファの更新
void Scene::updateLightUniformBuffer()
{
	//ディレクショナルライトの更新
	updateDirLightUniformBuffer();
	//ポイントライトの更新
	updatePointLightUniformBuffer();
	//シャドウマップの更新
	updateShadowMapUniformBuffer();
}

//ディレクショナルライトの更新
void Scene::updateDirLightUniformBuffer()
{
	if (sceneDirectionalLights.size() == 0)
	{
		return;
	}

	DirectionalLightUBO ubo{};

	int loopLimit = static_cast<int>(ubo.dir.size());
	if (loopLimit > static_cast<int>(sceneDirectionalLights.size()))
	{
		loopLimit = static_cast<int>(sceneDirectionalLights.size());
	}

	ubo.lightCount = loopLimit;

	for (int i = 0; i < loopLimit; i++)
	{
		ubo.dir[i] = glm::vec4(sceneDirectionalLights[i]->direction, 0.0f);
		ubo.color[i] = glm::vec4(sceneDirectionalLights[i]->color, 0.0f);
	}

	memcpy(dirLightBuffer.mappedBuffer.uniformBufferMapped, &ubo, sizeof(DirectionalLightUBO));
}

//ポイントライトの更新
void Scene::updatePointLightUniformBuffer()
{
	if (scenePointLights.size() == 0)
	{
		return;
	}

	PointLightUBO ubo{};

	int loopLimit = static_cast<int>(std::min(ubo.color.size(), scenePointLights.size()));

	ubo.lightCount = loopLimit;

	for (int i = 0; i < loopLimit; i++)
	{
		ubo.pos[i] = glm::vec4(scenePointLights[i]->getPosition(), 1.0f);
		ubo.color[i] = glm::vec4(scenePointLights[i]->color, 1.0f);
	}

	memcpy(pointLightBuffer.mappedBuffer.uniformBufferMapped, &ubo, sizeof(ubo));
}

//シャドウマップのユニフォームバッファの更新
void Scene::updateShadowMapUniformBuffer()
{
	Storage* storage = Storage::GetInstance();

	std::shared_ptr<Camera> camera = storage->accessCamera();

	for (int i = 0; i < sceneDirectionalLights.size(); i++)
	{
		shadowMapData.matUBOs[i].view = glm::lookAt(sceneDirectionalLights[i]->getPosition()
			, sceneDirectionalLights[i]->getPosition() + sceneDirectionalLights[i]->direction, glm::vec3(0.0f, 1.0f, 0.0f));

		shadowMapData.matUBOs[i].proj = shadowMapData.proj;

		memcpy(shadowMapData.mappedBuffers[i].uniformBufferMapped, &shadowMapData.matUBOs[i], sizeof(ShadowMapUBO));
	}
}

//gpu上のバッファを破棄する
//シーン管轄のインスタンスのバッファのみ破棄する
void Scene::cleanupVulkan()
{
	Storage* storage = Storage::GetInstance();

	VulkanBase* vulkan = VulkanBase::GetInstance();
	VkDevice device = vulkan->getDevice();

	//いったん処理が終わるまで待機する
	vulkan->gpuWaitIdle();

	shadowMapData.destroy(device);

	//モデル
	for (auto model : sceneModels)
	{
		model->cleanupVulkan();
	}
	player->cleanupVulkan();

	cubemap->cleanupVulkan();

	//UI
	for (auto ui : sceneUI)
	{
		ui->cleanupVulkan();
	}

	//ライト
	pointLightBuffer.mappedBuffer.destroy(device);
	dirLightBuffer.mappedBuffer.destroy(device);
}

void Scene::render()
{
	VulkanBase* vulkan = VulkanBase::GetInstance();

	//レンダリングの開始
	vulkan->renderBegin();

	//まずはシャドウマップの作成
	vulkan->shadowMapBegin(shadowMapData);

	for (auto model : sceneModels)
	{
		//半透明のオブジェクトの影は作らない
		if (!model->isTransparent() || true)
		{
			vulkan->renderShadowMap(model, shadowMapData);
		}
	}
	vulkan->renderShadowMap(player, shadowMapData);

	//シャドウマップの作製終了
	vulkan->shadowMapEnd();

	//3DモデルとUIのレンダリング開始
	vulkan->sceneRenderBegin();

	//半透明のオブジェクトのレンダリングは後に行う

	//UI
	for (auto ui : sceneUI)
	{
		if (!ui->isTransparent())
		{
			vulkan->renderUI(ui);
		}
	}

	//3Dモデル
	for (auto model : sceneModels)
	{
		if (!model->isTransparent())
		{
			vulkan->renderModel(model, cubemap
				, shadowMapData, pointLightBuffer, dirLightBuffer);
		}
	}

	//半透明のオブジェクトをレンダリングする

	//UI
	for (auto ui : sceneUI)
	{
		if (ui->isTransparent())
		{
			vulkan->renderUI(ui);
		}
	}

	//3Dモデル
	for (auto model : sceneModels)
	{
		if (model->isTransparent())
		{
			vulkan->renderModel(model, cubemap
				, shadowMapData, pointLightBuffer, dirLightBuffer);
		}
	}

	vulkan->renderModel(player, cubemap, shadowMapData, pointLightBuffer, dirLightBuffer);

	//キューブマップ
	vulkan->renderCubemap(cubemap);

	//3DモデルとUIのレンダリング終了
	vulkan->sceneRenderEnd();

	vulkan->renderEnd();
}