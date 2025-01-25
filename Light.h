#pragma once

#include"Object.h"

class Light :public Object
{
public:
	Light();

	glm::vec3 color;
};