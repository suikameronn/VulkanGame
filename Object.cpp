#include"Object.h"

void Object::setRotateData(RotateData* rotateData)
{
	this->rotateData = *rotateData;
}

void Object::setPos(glm::vec3& p)
{
	this->pos = p;
}

RotateData* Object::getRotateData()
{
	return &rotateData;
}

glm::vec3& Object::getPos()
{
	return pos;
}