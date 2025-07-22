#pragma once

#include<glm/glm.hpp>

struct Velocity
{
	glm::vec3 velocity; //‘¬“x

	Velocity()
	{
		velocity = glm::vec3(0.0f, 0.0f, 0.0f);
	}
};

struct PhysicComp
{
	//d—Í‰Á‘¬“x
	glm::vec3 gravity;
	//‹ó‹C’ïRŒW”
	float airResistance;
	//–€CŒW”
	float friction;

	PhysicComp()
	{
		gravity = glm::vec3(0.0f, -9.81f, 0.0f);
		airResistance = 0.1f;
		friction = 0.5f;
	};
};