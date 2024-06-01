#include"Camera.h"

Camera::Camera()
{
	uniformBufferChange = true;

	position = { 0,0.0f,1.0f };
	posOffSet = { 0.0f,0.0f,2.0f };

	forward = glm::vec3{ 0,0,-1 };
	right = glm::vec3{ 1,0,0 };

	theta = 95.8f;
	phi = 30.0f;
	viewAngle = 45;
	viewPointSpeed = 0.01f;
}

void Camera::setViewAngle(float f)
{
	viewAngle = f;
}

float Camera::getViewAngle()
{
	return viewAngle;
}

void Camera::Update()
{
	auto controller = Controller::GetInstance();

	if (otherObject)
	{
		auto controller = Controller::GetInstance();

		if (controller->getKey(GLFW_KEY_LEFT) != GLFW_RELEASE)
		{
			theta -= viewPointSpeed;
		}
		else if (controller->getKey(GLFW_KEY_RIGHT) != GLFW_RELEASE)
		{
			theta += viewPointSpeed;
		}
		else if (controller->getKey(GLFW_KEY_UP) != GLFW_RELEASE)
		{
			phi += viewPointSpeed;
		}
		else if (controller->getKey(GLFW_KEY_DOWN) != GLFW_RELEASE)
		{
			phi -= viewPointSpeed;
		}

		if (phi > 28.4f && phi < 31.3f)
		{
			setSpherePos(otherObject->getPosition(), 1.0f, theta, phi);
		}
	}
	else
	{
		auto controller = Controller::GetInstance();

		if (controller->getKey(GLFW_KEY_LEFT) != GLFW_RELEASE)
		{
			theta -= viewPointSpeed;
		}
		else if (controller->getKey(GLFW_KEY_RIGHT) != GLFW_RELEASE)
		{
			theta += viewPointSpeed;
		}
		else if (controller->getKey(GLFW_KEY_UP) != GLFW_RELEASE)
		{
			phi += viewPointSpeed;
		}
		else if (controller->getKey(GLFW_KEY_DOWN) != GLFW_RELEASE)
		{
			phi -= viewPointSpeed;
		}
	}
}

glm::vec3 Camera::getViewTarget()
{
	if (otherObject)
	{
		return otherObject->getPosition();
	}
	else
	{
		return position;
	}
}