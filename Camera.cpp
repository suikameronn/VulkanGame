#include"Camera.h"

Camera::Camera()
{
	uniformBufferChange = true;

	position = { 0,0.0f,0.0f };
	posOffSet = { 0.0f,0.0f,50.0f };

	forward = glm::vec3{ 0,0,1 };
	right = glm::vec3{ 1,0,0 };

	theta = 95.8f;
	phi = 30.0f;

	theta2 = theta;
	phi2 = phi;

	current = { 1.0, 0.0, 0.0, 0.0 };
	after = { 1.0, 0.0, 0.0, 0.0 };
	target = { 1.0,0.0,0.0,0.0 };
	quatMat = glm::mat4(1.0);

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
		
		if (!input)
		{
			phi2 = phi;
			theta2 = theta;
		}

		setSpherePos(theta, phi);
		setPosition(this->getBindObject()->getPosition() + posOffSet * glm::mat3(quatMat));
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