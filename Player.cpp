#include"Player.h"

Player::Player()
{
	forward = glm::vec3{ 0,0,-1 };
	right = glm::vec3{ 1,0,0 };

	speed = 0.1f;
	rotateSpeed = 0;
}

void Player::setObject(Model* obj)
{
	object = std::shared_ptr<Object>(obj);
}

Object* Player::getObject()
{
	return object.get();
}

void Player::inputKeys()
{
	auto controller = Controller::GetInstance();

	if (controller->getKey(GLFW_KEY_W))
	{
		moveDirec = forward;
	}
	else if (controller->getKey(GLFW_KEY_A))
	{
		moveDirec = -right;
	}
	else if (controller->getKey(GLFW_KEY_D))
	{
		moveDirec = right;
	}
	else if (controller->getKey(GLFW_KEY_S))
	{
		moveDirec = -forward;
	}
	else
	{
		moveDirec = { 0,0,0 };
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