#pragma once
#include"Object.h"
#include"Controller.h"

class Camera:public Object
{
private:
	float viewAngle;
	float viewPointSpeed;

	float theta, phi;

public:

	glm::mat3 mat;

	Camera();

	void setViewAngle(float f);
	float getViewAngle();

	glm::vec3 getViewTarget();

	void Update() override;
};