#include"PhysicBase.h"

PhysicBase::PhysicBase()
{
	gravity = glm::vec3(0.0, -0.1f, 0.0f);

	deltaTime = 0.0f;

	prevPos = glm::vec3(0.0f);
	currentPos = glm::vec3(0.0f);
	velocity = glm::vec3(0.0f);
	acceleration = glm::vec3(0.0f);
}

void PhysicBase::Update(bool isAccelerateReset)
{
	prevFrameTime = frameTime;
	prevPos = currentPos;

	frameTime = std::chrono::system_clock::now();

	velocity += gravity + acceleration * deltaTime;

	std::cout << velocity.y << std::endl;

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

glm::vec3 PhysicBase::getVelocity()
{
	return velocity;
}