#include"Object.h"

void Object::setRotateData(RotateData* rotateData)
{
	this->rotateData = *rotateData;
}

RotateData* Object::getRotateData()
{
	return &rotateData;
}