#pragma once

#define GLM_ENABLE_EXPERIMENTAL
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include<iostream>
#include<glm/glm.hpp>
#include<vector>

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

	glm::vec3 position;

public:
	bool uniformBufferChange;

	glm::vec3 posOffSet;
	glm::vec3 forward;
	glm::vec3 right;

	void bindObject(Object* obj);

	RotateData* getRotateData();

	void setPosition(glm::vec3 pos);
	glm::vec3 getPosition();

	virtual void Update();

	Object();
	virtual ~Object() {};
};