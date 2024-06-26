#include"Camera.h"

Camera::Camera()
{
	uniformBufferChange = true;

	position = { 0,0.0f,5.0f };
	posOffSet = { 0.0f,0.0f,50.0f };

	forward = glm::vec4{ 0,0,1,0 };
	right = glm::vec4{ 1,0,0,0 };
	up = glm::vec3{ 0,1,0 };

	axis = glm::vec3{ 0,1,0 };

	theta = 0.0f;
	phi = 0.0f;

	viewAngle = 45;
	viewPointSpeed = 1.0f;
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
		bool input = false;
		if (controller->getKey(GLFW_KEY_LEFT) != GLFW_RELEASE)
		{
			theta -= viewPointSpeed;
			input = true;
		}
		
		if (controller->getKey(GLFW_KEY_RIGHT) != GLFW_RELEASE)
		{
			theta += viewPointSpeed;
			input = true;
		}
		
		if (controller->getKey(GLFW_KEY_UP) != GLFW_RELEASE)
		{
			phi += viewPointSpeed;
			input = true;
		}
		
		if (controller->getKey(GLFW_KEY_DOWN) != GLFW_RELEASE)
		{
			phi -= viewPointSpeed;
			input = true;
		}

		if (phi >= 90.0f)
		{
			phi = 89.0f;
		}
		else if (phi <= -90.0f)
		{
			phi = -89.0f;
		}

		if (input)
		{
			setSpherePos(glm::radians(theta), glm::radians(phi));
		}
	}
	else
	{
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