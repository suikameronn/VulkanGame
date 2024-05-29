#include"Player.h"

Player::Player()
{
	forward = glm::vec3{ 0,0,-1 };
	right = glm::vec3{ 1,0,0 };

	speed = 0.1;
	rotateSpeed = 0;
}

void Player::setObject(Model* obj)
{
	object = obj;
}

Object* Player::getObject()
{
	return object;
}

void Player::inputKeys()
{
	auto controller = Controller::GetInstance();

	switch (controller->keyChar.charInfo)
	{
	case w:
		moveDirec = forward;
		break;
	case a:
		moveDirec = -right;
		break;
	case d:
		moveDirec = right;
		break;
	case s:
		moveDirec = -forward;
		break;
	default:
		moveDirec = { 0,0,0 };
		break;
	}

	if (controller->keyInput.key == GLFW_KEY_LEFT != GLFW_RELEASE)
	{
		std::cout << "left" << std::endl;
	}
	else if (controller->keyInput.key == GLFW_KEY_RIGHT != GLFW_RELEASE)
	{
		std::cout << "right" << std::endl;
	}
	else if (controller->keyInput.key == GLFW_KEY_UP != GLFW_RELEASE)
	{
		std::cout << "up" << std::endl;
	}
	else if (controller->keyInput.key == GLFW_KEY_DOWN != GLFW_RELEASE)
	{
		std::cout << "down" << std::endl;
	}
}

void Player::Update()
{
	inputKeys();
	
	glm::vec3 pos = { 0,0,0 };
	pos = moveDirec * speed;

	pos += object->getPosition();

	object->setPosition(pos);
}