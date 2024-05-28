#include"Object.h"

Object::Object()
{
	position = { 0,0,0 };

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

void Object::move()
{

}

void Object::Update()
{
	move();
}