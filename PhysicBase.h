#pragma once
#include<time.h>
#include<vector>
#include<memory>
#include<chrono>

#include<iostream>

#include<glm/glm.hpp>

class PhysicBase
{
private:

	glm::vec3 gravity;

	std::chrono::system_clock::time_point prevFrameTime;
	glm::vec3 prevPos;

	std::chrono::system_clock::time_point frameTime;

	float deltaTime;

	glm::vec3 currentPos;
	glm::vec3 velocity;
	glm::vec3 acceleration;

public:

	PhysicBase();

	virtual void Update(bool isAccelerateReset = true);

	void setZeroVelocity();

	virtual void setAcceleration(glm::vec3 acceleration);

	virtual void addVelocity(glm::vec3 addVelocity);

	virtual glm::vec3 getVelocity();
};