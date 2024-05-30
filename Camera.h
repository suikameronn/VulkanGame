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

	Camera();

	void setViewAngle(float f);
	float getViewAngle();

	void Update() override;
};