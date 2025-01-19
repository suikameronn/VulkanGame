#include"Camera.h"

Camera::Camera()
{
	position = { 0,0.0f,5.0f };
	distance = 100.0f;
	offsetPos = glm::vec3(0.0f,20.0f,0.0f);

	forward = glm::vec3{ 0,0,1};
	right = glm::vec3{ 1,0,0};
	up = glm::vec3{ 0,1,0 };

	theta = 0.0f;
	phi = 0.0f;

	viewAngle = 45;
	viewPointSpeed = 1.0f;

	setPosition(glm::vec3(0, 0, distance));

	perspectiveMat = glm::perspective(viewAngle, 800.0f / 600.0f, 0.1f, 10000.0f);
}

void Camera::setPosition(glm::vec3 pos)
{
	if (position == pos)
	{
		return;
	}

	position = pos;
}

void Camera::setDistance(float distance)
{
	this->distance = distance;
}

void Camera::setOffsetPos(glm::vec3 offset)
{
	this->offsetPos = offset;
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

void Camera::Update()
{
	customUpdate();
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

void Camera::setParentPos(glm::vec3 pos)
{
	parentPos = pos;
}

void Camera::setSpherePos(float theta, float phi)
{
	if (theta != 0.0f || phi != 0.0f || true)
	{
		glm::vec3 pos;
		pos = { distance * cos(theta) * cos(phi),distance * sin(phi),distance * sin(theta) * cos(phi) };
		pos += parentPos + offsetPos;

		this->forward = glm::normalize(glm::vec3(pos - parentPos));
		this->right = glm::cross(glm::vec3(0, 1, 0), this->forward);

		setPosition(pos);
	}
}

void Camera::calcViewMat()
{
	glm::vec3 pos = glm::vec3(parentPos.x, parentPos.y + 20.0f, parentPos.z);

	viewMat = glm::lookAt(this->position, pos/*parentPos*/, up);
}

glm::vec3 Camera::getViewTarget()
{
	return parentPos;
}

float Camera::getTheta()
{
	return theta;
}