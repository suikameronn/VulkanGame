#pragma once
#include<iostream>

#include<glm/glm.hpp>

class DirLight
{
public:
	DirLight();

	void setDirection(glm::vec3 d);
	void setColor(glm::vec3 c);

	glm::vec3 getDirection();
	glm::vec3 getColor();

private:
	glm::vec3 direction;
	glm::vec3 color;
};