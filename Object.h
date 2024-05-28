#pragma once

#define GLM_ENABLE_EXPERIMENTAL
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

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

	Object* otherObject;

	void move();

public:

	glm::vec3 position;
	glm::vec3 forward;
	glm::vec3 right;

	void bindObject(Object* obj);

	RotateData* getRotateData();

	virtual void Update();

	Object();
	virtual ~Object() {};
};