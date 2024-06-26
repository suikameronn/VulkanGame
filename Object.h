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

class Object
{
protected:

	Object* otherObject;
	float theta, phi;

	float rotateSpeed;
	float length;

	glm::quat current;
	glm::quat after;
	glm::quat target;

	glm::mat4 quatMat;
	void convertQuatMat();
	glm::quat makeQuat(glm::vec3 axis, float rad);
	glm::vec3 rotateQuatPosition(glm::vec3 pos, glm::vec3 axis, float rad);

	void move();

	glm::vec3 position;

	glm::vec3 inputMove();

public:

	Object();
	virtual ~Object() {};

	bool controllable;
	float speed;

	bool spherePos;

	bool uniformBufferChange;

	glm::vec3 posOffSet;
	glm::vec3 forward;
	glm::vec3 right;
	glm::vec3 up;

	glm::vec3 axis;

	void bindObject(Object* obj);
	Object* getBindObject() { return otherObject; }

	void setPosition(glm::vec3 pos);
	glm::vec3 getPosition();

	float convertRadian(float degree);
	glm::mat4 getQuatMat();

	void setSpherePos(float theta, float phi);

	virtual void updateTransformMatrix() {};
	virtual void Update();
};