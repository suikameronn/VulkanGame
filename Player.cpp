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
		right = cameraObj.lock()->right;
	}

	if (controller->getKey(GLFW_KEY_W))
	{
		moveDirec = -forward;
		rotate.y = 90.0f;
	}
	else if (controller->getKey(GLFW_KEY_A))
	{
		moveDirec = -right;
		rotate.y = 0.0f;
	}
	else if (controller->getKey(GLFW_KEY_D))
	{
		moveDirec = right;
		rotate.y = 180.0f;
	}
	else if (controller->getKey(GLFW_KEY_S))
	{
		moveDirec = forward;
		rotate.y = 270.0f;
	}
	else
	{
		moveDirec = { 0,0,0 };
	}

	if (moveDirec != glm::vec3(0.0f))
	{
		moveDirec = glm::normalize(moveDirec) * moveSpeed;
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

	if (spherePos)
	{
		setSpherePos(theta, phi);
	}
}