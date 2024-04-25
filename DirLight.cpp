#include"DirLight.h"

DirLight::DirLight()
{
	direction = { 0,0,0 };
	color = { 0.0,0.0,0.0 };
}

void DirLight::setDirection(glm::vec3 d)
{
	direction = d;
}

void DirLight::setColor(glm::vec3 c)
{
	color = c;
}

glm::vec3 DirLight::getDirection()
{
	return direction;
}

glm::vec3 DirLight::getColor()
{
	return color;
}