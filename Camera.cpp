#include"Camera.h"

Camera::Camera()
{
	uniformBufferChange = true;

	position = { 0,0.0f,1.0f };
	posOffSet = { 0,0.5,3 };

	forward = glm::vec3{ 0,0,-1 };
	right = glm::vec3{ 1,0,0 };

	theta = 0.0f;
	phi = 0.0f;
	viewAngle = 45;
	viewPointSpeed = 0.1f;
}

void Camera::setViewAngle(float f)
{
	viewAngle = f;
}

float Camera::getViewAngle()
{
	return viewAngle;
}

void Camera::sphereMove()
{
	auto controller = Controller::GetInstance();

	if (controller->keyInput.key == GLFW_KEY_LEFT != GLFW_RELEASE)
	{
		theta -= viewPointSpeed;
	}
	else if (controller->keyInput.key == GLFW_KEY_RIGHT != GLFW_RELEASE)
	{
		theta += viewPointSpeed;
	}
	else if (controller->keyInput.key == GLFW_KEY_UP != GLFW_RELEASE)
	{
		phi -= viewPointSpeed;
	}
	else if (controller->keyInput.key == GLFW_KEY_DOWN != GLFW_RELEASE)
	{
		phi += viewPointSpeed;
	}

	float r = glm::length(posOffSet);

	glm::vec3 pos = { r * sin(phi) * cos(theta) ,r * cos(phi),-r * sin(phi) * sin(theta) };
	pos += otherObject->getPosition();
	setPosition(pos);
}

void Camera::Update()
{
	auto controller = Controller::GetInstance();

	if (otherObject)
	{
		sphereMove();
	}
	else
	{
		if (controller->keyInput.key == GLFW_KEY_LEFT != GLFW_RELEASE)
		{
			std::cout << "left" << std::endl;
		}
		else if (controller->keyInput.key == GLFW_KEY_RIGHT != GLFW_RELEASE)
		{
			std::cout << "right" << std::endl;
		}
		else if (controller->keyInput.key == GLFW_KEY_UP != GLFW_RELEASE)
		{
			std::cout << "up" << std::endl;
		}
		else if (controller->keyInput.key == GLFW_KEY_DOWN != GLFW_RELEASE)
		{
			std::cout << "down" << std::endl;
		}
	}
}