#pragma once
#include"Object.h"

class Camera:public Object
{
private:
	float viewAngle;

public:

	Camera();

	void setViewAngle(float f);
	float getViewAngle();
};