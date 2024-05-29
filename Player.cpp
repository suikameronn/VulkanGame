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
}

void Player::Update()
{
	inputKeys();
	
	glm::vec3 pos = { 0,0,0 };
	pos = moveDirec * speed;

	pos += object->getPosition();

	object->setPosition(pos);
}