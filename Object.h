#pragma once

#define GLM_ENABLE_EXPERIMENTAL
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#define PI 3.14159265359

#include<iostream>
#include<vector>
#include<math.h>

#include<glm/glm.hpp>
#include<glm/ext.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include"EnumList.h"

class Camera;

class Object
{
protected:

	Object* parentObject;
	std::vector<Object*> childObjects;
	float theta, phi;

	std::shared_ptr<Camera> bindCamera;

	float rotateSpeed;
	float length;

	glm::vec3 pivot;
	glm::mat4 pivotMatrix;
	glm::vec3 position;

	virtual glm::vec3 inputMove();

public:

	Object();

	bool controllable;
	float speed;

	bool spherePos;

	bool uniformBufferChange;

	float posOffSet;
	glm::vec3 forward;
	glm::vec3 right;
	glm::vec3 up;

	glm::vec3 axis;

	glm::mat4 transformMatrix;

	void bindObject(Object* obj);
	void bindObject(std::shared_ptr<Camera> camera);
	void setParentObject(Object* obj);

	void setPosition(glm::vec3 pos);
	glm::vec3 getPosition();

	virtual glm::vec3 getPivot() { return pivot; }

	void setSpherePos(float theta, float phi);

	glm::mat4 getTransformMatrix();

	virtual void updateTransformMatrix() {};
	virtual void Update();
};