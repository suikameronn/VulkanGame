#pragma once

#include<glm/glm.hpp>
#include<vector>
#include <stdexcept>

struct RotateData
{
	glm::vec3 direction;
	float rad;
};

class Object
{
protected:
	RotateData rotateData;

public:
	void setRotateData(RotateData* rotateData);

	RotateData* getRotateData();
};