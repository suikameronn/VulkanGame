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
	theta = 30.0f;
	phi = 30.0f;
	rotateSpeed = 0.1f;
}

void Object::bindObject(Object* obj)
{
	otherObject = obj;
}

void Object::Update()
{

}

void Object::setPosition(glm::vec3 pos)
{
	position = pos;
}

glm::vec3 Object::getPosition()
{
	return position;
}

void Object::setSpherePos(glm::vec3 center,float r, float theta, float phi)
{
	glm::vec3 pos = { r * sin(phi) * cos(theta) ,r * cos(phi),-r * sin(phi) * sin(theta) };

	pos += center;
	setPosition(pos);
}