#include"Camera.h"

Camera::Camera()
{
	uniformBufferChange = true;

	position = { 0,0.0f,5.0f };
	posOffSet = 50.0f;

	spherePos = true;

	forward = glm::vec3{ 0,0,1};
	right = glm::vec3{ 1,0,0};
	up = glm::vec3{ 0,1,0 };

	axis = glm::vec3{ 0,1,0 };

	theta = 0.0f;
	phi = 0.0f;

	childObjects.clear();

	viewAngle = 45;
	viewPointSpeed = 1.0f;

	setPosition(glm::vec3(0, 0, posOffSet));

	perspectiveMat = glm::perspective(viewAngle, 800.0f / 600.0f, 0.1f, 10000.0f);
}

void Camera::setViewAngle(float f)
{
	viewAngle = f;

	perspectiveMat = glm::perspective(viewAngle, 800.0f / 600.0f, 0.1f, 10000.0f);
}

float Camera::getViewAngle()
{
	return viewAngle;
}

void Camera::customUpdate()
{
	auto controller = Controller::GetInstance();

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

	setSpherePos(glm::radians(theta), glm::radians(phi));
	calcViewMat();
}

void Camera::calcViewMat()
{
	viewMat = glm::lookAt(this->position, parentPos , up);
}

glm::vec3 Camera::getViewTarget()
{
	return parentPos;
}