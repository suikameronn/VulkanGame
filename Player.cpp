#include "Player.h"

Player::Player()
{
	controllable = true;
	moveSpeed = 1.0f;
}

Player::Player(std::string luaScriptPath)
{
	controllable = true;
	moveSpeed = 1.0f;
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
		switchPlayAnimation("Idle");
	}

	if (controller->getKey(GLFW_KEY_SPACE))
	{
		if (boxRayCast(position + up * 10.0f,glm::vec3(1.0f,0.0,0.0f), 100.0f))
		{
			physicBase->addVelocity(10.0f * up);
		}
		else
		{
			std::cout << "No hit" << std::endl;
		}
	}

	if (moveDirec != glm::vec3(0.0f))
	{
		moveDirec = glm::normalize(moveDirec) * moveSpeed;
		switchPlayAnimation("Running");
	}

	return moveDirec;
}

void Player::customUpdate()
{
	if (gltfModel->animations.size() > 0)
	{
		playAnimation();
	}

	if (controllable)
	{
		glm::vec3 moveDirec = inputMove();
		setPosition(this->position + moveDirec);
	}
}