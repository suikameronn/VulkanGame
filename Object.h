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
	glm::vec3 pos;
	RotateData rotateData;

protected:
	void setRotateData(RotateData* rotateData);
	void setPos(glm::vec3& p);

	glm::vec3& getPos();
	RotateData* getRotateData();
};