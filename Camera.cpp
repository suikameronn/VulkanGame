#include"Camera.h"

Camera::Camera()
{
	uniformBufferChange = true;

	position = { 0,0.0f,1.0f };
	posOffSet = { 0,0.5,3 };

	forward = glm::vec3{ 0,0,-1 };
	right = glm::vec3{ 1,0,0 };

	viewAngle = 45;
}

void Camera::setViewAngle(float f)
{
	viewAngle = f;
}

float Camera::getViewAngle()
{
	return viewAngle;
}