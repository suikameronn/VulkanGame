#include "Player.h"

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

void Player::Update()
{
	if (gltfModel->animations.size() > 0)
	{
		playAnimation();
	}

	if (controllable)
	{
		glm::vec3 moveDirec = inputMove();
		setPosition(this->position + moveDirec * speed);
	}

	if (spherePos)
	{
		setSpherePos(theta, phi);
	}
}