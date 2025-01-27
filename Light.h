#pragma once

#include"Object.h"
#include<vulkan/vulkan.h>

class PointLight :public Object
{
public:
	PointLight();

	glm::vec3 color;
};

class DirectionalLight:public Object
{
public:
	DirectionalLight();

	glm::vec3 color;
	glm::vec3 direction;
};