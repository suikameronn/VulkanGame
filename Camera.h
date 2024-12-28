#pragma once

#define GLM_ENABLE_EXPERIMENTAL

#include<glm/glm.hpp>
#include<glm/ext.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include"Controller.h"

class Camera
{
private:
	glm::vec3 position;
	glm::vec3 parentPos;
	float posOffSet;

	float viewAngle;
	float viewPointSpeed;

	float theta, phi;

	void calcViewMat();
public:
	glm::vec3 forward, right, up;

	glm::mat4 viewMat;
	glm::mat4 perspectiveMat;

	Camera();

	void setPosition(glm::vec3 pos);
	void setParentPos(glm::vec3 pos);
	void setSpherePos(float theta, float phi);

	void setViewAngle(float f);
	float getViewAngle();
	float getTheta();

	glm::vec3 getViewTarget();

	void Update();
	void customUpdate();
};