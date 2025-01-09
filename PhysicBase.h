#pragma once
#include<time.h>

#include<glm/glm.hpp>

class PhysicBase
{
private:
	float prevFrameTime;
	glm::vec3 prevPos;

	float frameTime;

	float deltaTime;

	glm::vec3 currentPos;
	glm::vec3 velocity;
	glm::vec3 acceleration;

public:

	PhysicBase();

	virtual void Update(float time, bool isAccelerateReset = true);

	virtual void setVelocity(glm::vec3 velocity);
	void setZeroVelocity();

	virtual void setAcceleration(glm::vec3 acceleration);

	virtual void addVelocity(glm::vec3 addVelocity);

	virtual glm::vec3 getAddPos();

	virtual glm::vec3 getVelocity();
};