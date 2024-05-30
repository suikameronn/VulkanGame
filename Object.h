#pragma once

#define GLM_ENABLE_EXPERIMENTAL
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include<iostream>
#include<glm/glm.hpp>
#include<vector>
#include<math.h>

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
	float theta, phi;
	float rotateSpeed;

	void move();

	glm::vec3 position;

public:
	bool spherePos;

	bool uniformBufferChange;

	glm::vec3 posOffSet;
	glm::vec3 forward;
	glm::vec3 right;

	void bindObject(Object* obj);

	RotateData* getRotateData();

	void setPosition(glm::vec3 pos);
	glm::vec3 getPosition();

	void setSpherePos(glm::vec3 center,glm::vec3 posOffSet, float theta, float phi);

	virtual void Update();

	Object();
	virtual ~Object() {};
};