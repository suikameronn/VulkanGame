#include"PhysicBase.h"

PhysicBase::PhysicBase()
{
	gravity = glm::vec3(0.0, -0.3f, 0.0f);

	velocity = glm::vec3(0.0f);
}

void PhysicBase::Update(bool isAccelerateReset)
{
	velocity += gravity;
}

void PhysicBase::setZeroVelocity()
{
	this->velocity = glm::vec3(0.0f);
}

void PhysicBase::addVelocity(glm::vec3 addVelocity)
{
	this->velocity += addVelocity;
}

glm::vec3 PhysicBase::getVelocity()
{
	return velocity;
}

void PhysicBase::cancelGravity()
{
	velocity.y = -0.3f;
}

void PhysicBase::addGravity()
{
	addVelocity(gravity);
}