#include"Object.h"
#include"Controller.h"

Object::Object()
{
	uniformBufferChange = true;

	position = { 0,0,0 };
	posOffSet = { 0,0,0 };

	forward = glm::vec3{ 0,0,1 };
	right = glm::vec3{ 1,0,0 };

	otherObject = nullptr;
	spherePos = false;

	controllable = false;
	speed = 10.0f;

	rotateSpeed = 0.1f;
	length = 1.0f;
}

float Object::convertRadian(float degree)
{
	return degree * (PI / 180);
}

void Object::bindObject(Object* obj)
{
	otherObject = obj;
}

glm::vec3 Object::inputMove()
{
	glm::vec3 moveDirec;
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

	return moveDirec;
}

void Object::Update()
{
	if (controllable)
	{
		glm::vec3 moveDirec = inputMove();
		setPosition(this->position + moveDirec * speed);
	}
}

void Object::setPosition(glm::vec3 pos)
{
	if (otherObject != nullptr)
	{

	}

	position = pos;
}

glm::vec3 Object::getPosition()
{
	return position;
}

void Object::setSpherePos(float theta, float phi)
{
	glm::vec3 pos;
	pos = { 200.0f * cos(theta) * cos(phi),200.0f * sin(phi),200.0f * sin(theta) * cos(phi) };

	setPosition(pos);
}