#include"Object.h"

Object::Object()
{
	uniformBufferChange = true;

	position = { 0,0,0 };
	posOffSet = { 0,0,0 };

	forward = glm::vec3{ 0,0,1 };
	right = glm::vec3{ 1,0,0 };
}

void Object::bindObject(Object* obj)
{
	otherObject = obj;
}

RotateData* Object::getRotateData()
{
	return &rotateData;
}

void Object::Update()
{
	glm::vec3 pos = { 0,0,0 };
	setPosition(pos);
}

void Object::setPosition(glm::vec3 pos)
{
	if (otherObject)
	{
		position = posOffSet + otherObject->getPosition();
	}
	else
	{
		position = pos;
	}
}

glm::vec3 Object::getPosition()
{
	return position;
}