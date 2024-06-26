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

	theta2 = theta;
	phi2 = phi;

	current = { 1.0, 0.0, 0.0, 0.0 };
	after = { 1.0, 0.0, 0.0, 0.0 };
	target = { 1.0,0.0,0.0,0.0 };
	quatMat = glm::mat4(1.0);

	viewAngle = 45;
	viewPointSpeed = 20.0f;
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

	theta = 0;
	phi = 0;
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

		setSpherePos(glm::radians(theta), glm::radians(phi));
		//setPosition(this->getBindObject()->getPosition() + posOffSet * glm::mat3(quatMat));
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