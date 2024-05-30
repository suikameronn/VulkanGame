#include"Object.h"

Object::Object()
{
	uniformBufferChange = true;

	position = { 0,0,0 };
	posOffSet = { 0,0,0 };

	forward = glm::vec3{ 0,0,1 };
	right = glm::vec3{ 1,0,0 };

	otherObject = nullptr;
	spherePos = false;
	theta = 0.01f;
	phi = 0.01f;
	rotateSpeed = 0.1f;
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
	glm::vec3 pos = { 0,0,-10 };


	setPosition(pos);
}

void Object::setPosition(glm::vec3 pos)
{
	position = pos;
}

glm::vec3 Object::getPosition()
{
	return position;
}

void Object::setSpherePos(glm::vec3 center,glm::vec3 posOffSet, float theta, float phi)
{
	float r = glm::length(posOffSet);

	glm::vec3 pos = { r * sin(phi) * cos(theta) ,r * cos(phi),-r * sin(phi) * sin(theta) };
	pos += center;
	setPosition(pos);
}