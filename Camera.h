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
	
	float distance;
	glm::vec3 offsetPos;

	float viewAngle;
	float viewPointSpeed;

	float theta, phi;

	float zNear, zFar;

	void calcViewMat();
public:
	glm::vec3 forward, right, up;

	glm::mat4 viewMat;
	glm::mat4 perspectiveMat;

	Camera();

	void setPosition(glm::vec3 pos);
	glm::vec3 getPosition() { return position; }
	void setParentPos(glm::vec3 lastPos, glm::vec3 position);
	void setDistance(float distance);
	void setOffsetPos(glm::vec3 offset);

	void setSpherePos(float theta, float phi);

	void setViewAngle(float f);
	float getViewAngle();
	float getTheta();

	void getzNearFar(float& near, float& far);

	glm::vec3 getViewTarget();

	void Update();
	void customUpdate();
};