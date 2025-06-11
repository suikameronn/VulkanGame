#include"Scene.h"
#include"VulkanBase.h"
#include"Bullet.h"

#include "Player.h"

#include"GameManager.h"

Player::Player()
{
	objNum = ObjNum::PLAYER;

	aiming = false;

	controllable = true;
	moveSpeed = 1.0f;

	physicBase = std::make_unique<PhysicBase>();

	bulletFilePath = "beamBullet.glb";
	bulletSpeed = 1.0f;
	rayCastLength = 1.0f;
	bulletDistanceLimit = 1000.0f;
}

Player::Player(std::string luaScriptPath)
{
	aiming = false;

	controllable = true;
	moveSpeed = 1.0f;

	physicBase = std::make_unique<PhysicBase>();

	bulletFilePath = "beamBullet.glb";
	bulletSpeed = 1.0f;
	rayCastLength = 1.0f;
	bulletDistanceLimit = 1000.0f;
}

//luaから呼び出される関数を登録される
void Player::registerGlueFunctions()
{
	lua_register(lua, "setSpeed", glueSetSpeed);//移動速度の設定
	lua_register(lua, "setJumpHeight", glueSetJumpHeight);//ジャンプの高さの設定
	lua_register(lua, "setTargetUI", glueSetTargetUI);//照準UIの画像とサイズを設定
	lua_register(lua, "setAimingCameraPos", glueSetAimingCameraPos);//狙いを定めた時のカメラの位置の設定
}

//キー入力からプレイヤーを移動させる
glm::vec3 Player::inputMove()
{
	glm::vec3 moveDirec;
	auto controller = Controller::GetInstance();

	if (!cameraObj.expired())
	{
		forward = cameraObj.lock()->forward;
		forward.y = 0.0f;
		right = cameraObj.lock()->right;
		cameraDirDeg = cameraObj.lock()->getTheta();
	}

	if (!aiming)
	{
		if (controller->getKey(GLFW_KEY_W))
		{
			moveDirec = forward;
			rotate.y = cameraDirDeg;
		}
		else if (controller->getKey(GLFW_KEY_A))
		{
			moveDirec = -right;
			rotate.y = cameraDirDeg - 90.0f;
		}
		else if (controller->getKey(GLFW_KEY_D))
		{
			moveDirec = right;
			rotate.y = cameraDirDeg + 90.0f;
		}
		else if (controller->getKey(GLFW_KEY_S))
		{
			moveDirec = -forward;
			rotate.y = cameraDirDeg + 180.0f;
		}
		else
		{
			moveDirec = { 0,0,0 };
		}

		if (moveDirec == glm::vec3(0.0f))
		{
			switchPlayAnimation("Idle_gunMiddle");
		}
		else
		{
			moveDirec = glm::normalize(moveDirec) * moveSpeed;
			switchPlayAnimation("run");
		}
	}
	else
	{
		if (controller->getKey(GLFW_KEY_W))
		{
			moveDirec = forward;
			switchPlayAnimation("walk_back_shoot");
		}
		else if (controller->getKey(GLFW_KEY_A))
		{
			moveDirec = -right;
			switchPlayAnimation("walk_left_shoot");
		}
		else if (controller->getKey(GLFW_KEY_D))
		{
			moveDirec = right;
			switchPlayAnimation("walk_right_shoot");
		}
		else if (controller->getKey(GLFW_KEY_S))
		{
			moveDirec = -forward;
			switchPlayAnimation("walk_front_shoot");
		}
		else
		{
			moveDirec = { 0,0,0 };
		}

		if (moveDirec == glm::vec3(0.0f))
		{
			switchPlayAnimation("aiming");
		}
		else
		{
			moveDirec = glm::normalize(moveDirec) * moveSpeed;
		}
	}

	glm::vec3 groundNormal;
	if (isGround(groundNormal))
	{
		physicBase->cancelGravity();

		if (controller->getKey(GLFW_KEY_SPACE))
		{
			physicBase->addVelocity(up * maxJumpHeight);
		}
	}
	else
	{
		physicBase->addGravity();
	}

	return moveDirec;
}

//初回フレームのみ実行
void Player::initFrameSetting()
{
	if (lua)
	{
		registerGlueFunctions();

		lua_pushlightuserdata(lua, this);
		lua_setglobal(lua, "Data");

		luaL_dofile(lua, luaPath.c_str());
	}

	if (defaultAnimationName != "none")
	{
		currentPlayAnimationName = defaultAnimationName;
		nextPlayAnimationName = defaultAnimationName;
	}

	if (hasColiderFlag)
	{
		colider = std::shared_ptr<Colider>(new Colider(gltfModel, trigger));
		colider->initFrameSettings(glm::vec3(1.0f));
	}

	initMin = min;
	initMax = max;

	//アニメーション行列の初期化
	for (auto& matrices : jointMatrices)
	{
		std::fill(matrices.begin(), matrices.end(), glm::mat4(1.0f));
	}

	updateTransformMatrix();

	//Rツリーにオブジェクトを追加
	scene->addModelToRTree(std::dynamic_pointer_cast<Model>(shared_from_this()));

	aimingCameraOffset = aimingCameraOffsetSrc;

	//カメラの位置を調整
	sendPosToCamera(((min + max) / 2.0f));

	//gpu上にバッファを作成
	VulkanBase::GetInstance()->setModelData(std::static_pointer_cast<Model>(shared_from_this()));
}

//キー入力の取得
void Player::customUpdate()
{
	Controller* controller = Controller::GetInstance();

	glm::vec3 moveDirec = inputMove();

	setPosition(this->position + moveDirec);

	if (controller->getMouseButton(GLFW_MOUSE_BUTTON_RIGHT))
	{
		aiming = true;
		aim();
	}
	else if(aiming)
	{
		aiming = false;

		//カメラの位置をリセット
		cameraObj.lock()->posReset();
		//カメラの球面移動を開始
		cameraObj.lock()->sphereMove = true;
		//カメラの追従させる座標をこのオブジェクトの中心にする
		cameraObj.lock()->setParentPos(((min + max) / 2.0f) - cameraObj.lock()->getViewTarget());

		targetUI->setVisible(false);

		//自分を半透明でレンダリングするのをやめる
		fragParam.alphaness = -1.0f;
	}
}

//移動速度を設定する
void Player::setSpeed(float s)
{
	moveSpeed = s;
}

//ジャンプの高さを設定する
void Player::setMaxJumpHeight(float height)
{
	maxJumpHeight = height;
}

//リスタート地点へプレイヤーをワープさせる
void Player::restart(glm::vec3 startPoint)
{
	setZeroVelocity();

	rotate.x = rotate.getRadian(0.0f);
	rotate.y = rotate.getRadian(0.0f);
	rotate.z = rotate.getRadian(0.0f);

	setPosition(startPoint);
}

//ターゲットUIの画像とサイズを設定
void Player::setTargetUIImageAndScale(std::string filePath, float scale)
{
	GameManager* manager = GameManager::GetInstance();

	targetImage = FileManager::GetInstance()->loadImage(filePath);

	targetUI = std::shared_ptr<UI>(new UI(targetImage));
	targetUI->setScale(scale);
	targetUI->setPosition(glm::vec2(manager->getWindowWidth() / 2.0f, manager->getWindowHeight() / 2.0f));
	targetUI->setVisible(false);//デフォルトで非表示に指定おく
	targetUI->setTransparent(true);

	scene->sceneUI.push_back(targetUI);
}

void Player::Update()
{
	setLastFrameTransform();

	customUpdate();//オブジェクト固有の更新処理

	setPosition(getPosition() + physicBase->getVelocity());

	playAnimation();//アニメーションの再生
}

//座標変換行列の更新
void Player::updateTransformMatrix()
{
	glm::mat4 tr = glm::translate(glm::mat4(1.0), position) * rotate.getRotateMatrix();
	transformMatrix = tr * glm::scale(glm::mat4(1.0f), initScale * scale);

	//AABBの更新
	min = transformMatrix * glm::vec4(initMin, 1.0f);
	max = transformMatrix * glm::vec4(initMax, 1.0f);

	//狙いを定めた時のカメラの位置を調整
	aimingCameraOffset = tr * glm::vec4(aimingCameraOffsetSrc, 1.0f);

	//MBRの更新
	calcMBR();

	pivot = (mbrMin + mbrMax) / 2.0f;

	if (colider)
	{
		colider->reflectMovement(transformMatrix);
	}

	uniformBufferChange = false;

	if (rNode)
	{
		//Rツリー上のオブジェクトの位置を更新する
		scene->updateObjectPos(std::dynamic_pointer_cast<Model>(shared_from_this()), rNode);
	}
}

void Player::aim()
{
	Controller* controller = Controller::GetInstance();

	//照準UIの表示
	targetUI->setVisible(true);

	//カメラを所定の位置に設定する
	cameraObj.lock()->setPosition(aimingCameraOffset);

	cameraObj.lock()->setParentPos((aimingCameraOffset + forward * 1.1f) - cameraObj.lock()->getViewTarget());

	cameraObj.lock()->sphereMove = false;

	{
		//プレイヤーの向きを狙いを定めた方向に向ける

		//y軸
		float yaw = atan2(-forward.z, -forward.x);
		//x軸
		float pitch = asin(-forward.y);
		//z軸は回転させない
		float roll = 0.0f;

		//度に変換しておく
		rotate.x = pitch * (180.0f / glm::pi<float>());
		rotate.y = yaw * (180.0f / glm::pi<float>());
		rotate.z = roll * (180.0f / glm::pi<float>());
	}

	//自分自身を半透明でレンダリングするようにする
	fragParam.alphaness = 0.7f;

	if (controller->getMouseButton(GLFW_MOUSE_BUTTON_LEFT))
	{
		shootBullet();
	}
}

void Player::shootBullet()
{
	VulkanBase* vulkan = VulkanBase::GetInstance();

	//弾の進行方向
	bulletDirection = forward;

	std::shared_ptr bullet =
		std::shared_ptr<Bullet>(new Bullet(bulletSpeed, rayCastLength
			, bulletDirection, pivot, bulletDistanceLimit));

	//スケールの設定
	bullet->scale = glm::vec3(20.0f, 1.0f, 1.0f);

	//3Dモデルを設定
	bullet->setgltfModel(Storage::GetInstance()->getgltfModel(bulletFilePath));

	//コライダーを設定(トリガーの設定にする)
	bullet->setColider(true);

	//レンダリングのリストに追加
	scene->sceneModels.push_back(bullet);
}