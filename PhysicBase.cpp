#include"PhysicBase.h"

PhysicBase::PhysicBase()
{
	prevFrameTime = 0.0f;
	frameTime = 0.0f;
	deltaTime = 0.0f;

	prevPos = glm::vec3(0.0f);
	currentPos = glm::vec3(0.0f);
	velocity = glm::vec3(0.0f);
	acceleration = glm::vec3(0.0f);
}

void PhysicBase::Update(float time, bool isAccelerateReset)
{
	prevFrameTime = frameTime;
	prevPos = currentPos;

	frameTime = time;

	deltaTime = frameTime - prevFrameTime;

	getAddPos();

	velocity += acceleration * deltaTime;

	if (isAccelerateReset)
	{
		acceleration = glm::vec3(0.0f);
	}


}

void PhysicBase::setZeroVelocity()
{
	this->velocity = glm::vec3(0.0f);
	this->acceleration = glm::vec3(0.0f);
}

void PhysicBase::setAcceleration(glm::vec3 acceleration)
{
	this->acceleration = acceleration;
}

void PhysicBase::addVelocity(glm::vec3 addVelocity)
{
	this->velocity += addVelocity;
}

glm::vec3 PhysicBase::getAddPos()
{
	return currentPos + velocity;
}

glm::vec3 PhysicBase::getVelocity()
{
	return velocity;
}