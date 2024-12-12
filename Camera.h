#pragma once
#include"Object.h"
#include"Controller.h"

class Camera:public Object
{
private:
	float viewAngle;
	float viewPointSpeed;

	void calcViewMat();
public:

	glm::mat4 viewMat;
	glm::mat4 perspectiveMat;

	Camera();

	void setViewAngle(float f);
	float getViewAngle();

	glm::vec3 getViewTarget();

	void customUpdate() override;
};