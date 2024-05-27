#include"Object.h"

Object::Object()
{
	controllable = false;
	position = { 0,0,0 };
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