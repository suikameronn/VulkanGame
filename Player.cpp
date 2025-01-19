#include "Player.h"

Player::Player()
{
	jumpHeight = 0.0f;
	controllable = true;
	moveSpeed = 1.0f;

	physicBase = std::make_unique<PhysicBase>();
}

Player::Player(std::string luaScriptPath)
{
	jumpHeight = 0.0f;
	controllable = true;
	moveSpeed = 1.0f;

	physicBase = std::make_unique<PhysicBase>();
}

void Player::registerGlueFunctions()
{
	lua_register(lua, "setSpeed", glueSetSpeed);
	lua_register(lua, "setJumpHeight", glueSetJumpHeight);
}

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
		gravity = 1.0f;
		physicBase->setZeroVelocity();

		if (controller->getKey(GLFW_KEY_SPACE))
		{
			jumpHeight += 1.0f;
		} 
		

		if (jumpHeight > 0.0f)
		{
			if ((!controller->getKey(GLFW_KEY_SPACE)) || jumpHeight >= maxJumpHeight)
			{
				physicBase->addVelocity(up * maxJumpHeight);
				jumpHeight = 0.0f;
			}
		}
	}

	return moveDirec;
}

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

void Player::customUpdate()
{
	if (gltfModel->animations.size() > 0)
	{
		playAnimation();
	}

	glm::vec3 moveDirec = inputMove();
	setPosition(this->position + moveDirec);
}

void Player::setSpeed(float s)
{
	moveSpeed = s;
}

void Player::setMaxJumpHeight(float height)
{
	maxJumpHeight = height;
}