#include "Player.h"

Player::Player()
{
	controllable = true;
	moveSpeed = 1.0f;

	physicBase = std::make_unique<PhysicBase>();
}

Player::Player(std::string luaScriptPath)
{
	controllable = true;
	moveSpeed = 1.0f;

	physicBase = std::make_unique<PhysicBase>();
}

//luaから呼び出される関数を登録される
void Player::registerGlueFunctions()
{
	lua_register(lua, "setSpeed", glueSetSpeed);//移動速度の設定
	lua_register(lua, "setJumpHeight", glueSetJumpHeight);//ジャンプの高さの設定
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

	if (controller->getKey(GLFW_KEY_W))
	{
		moveDirec = -forward;
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
		moveDirec = forward;
		rotate.y = cameraDirDeg + 180.0f;
	}
	else
	{
		moveDirec = { 0,0,0 };
	}

	if (moveDirec == glm::vec3(0.0f))
	{
		switchPlayAnimation("Idle");
	}
	else
	{
		moveDirec = glm::normalize(moveDirec) * moveSpeed;
		switchPlayAnimation("Running");
	}

	if (isGround())
	{
		physicBase->setZeroVelocity();

		if (controller->getKey(GLFW_KEY_SPACE))
		{
			physicBase->addVelocity(up * maxJumpHeight);
		}
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
		switchPlayAnimation();
	}

	if (colider)
	{
		colider->initFrameSettings();
	}
}

//キー入力の取得
void Player::customUpdate()
{
	if (gltfModel->animations.size() > 0)
	{
		playAnimation();
	}

	glm::vec3 moveDirec = inputMove();
	setPosition(this->position + moveDirec);
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