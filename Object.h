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
	bool controllable;

	glm::vec3 position;
	RotateData rotateData;

	Object* otherObject;

	void move();
	virtual void Update();

public:

	void bindObject(Object* obj);

	glm::vec3* getPosition();
	RotateData* getRotateData();

	Object();
	virtual ~Object() {};
};