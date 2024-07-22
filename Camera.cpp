#include"Camera.h"
#include"VulkanBase.h"

Camera::Camera()
{
	uniformBufferChange = true;

	position = { 0,0.0f,5.0f };
	posOffSet = 200.0f;

	forward = glm::vec3{ 0,0,1};
	right = glm::vec3{ 1,0,0};
	up = glm::vec3{ 0,1,0 };

	axis = glm::vec3{ 0,1,0 };

	theta = 0.0f;
	phi = 0.0f;

	parentObject = nullptr;
	childObjects.clear();

	viewAngle = 45;
	viewPointSpeed = 1.0f;

	setPosition(glm::vec3(0, 0, posOffSet));

	perspectiveMat = glm::perspective(viewAngle, VulkanBase::GetInstance()->getAspect(), 0.1f, 1000.0f);
}

void Camera::setViewAngle(float f)
{
	viewAngle = f;

	perspectiveMat = glm::perspective(viewAngle, VulkanBase::GetInstance()->getAspect(), 0.1f, 1000.0f);
}

float Camera::getViewAngle()
{
	return viewAngle;
}

void Camera::Update()
{
	auto controller = Controller::GetInstance();

	if (parentObject)
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

		setSpherePos(glm::radians(theta), glm::radians(phi));
		calcViewMat();
	}
	else
	{
	}
}

void Camera::calcViewMat()
{
	std::cout << this->parentObject->getPosition().x << " " << this->parentObject->getPosition().y << " " << this->parentObject->getPosition().z << std::endl;
	viewMat = glm::lookAt(this->position, this->parentObject->getPosition(), glm::vec3(0, 1, 0));
}

glm::vec3 Camera::getViewTarget()
{
	if (parentObject)
	{
		return parentObject->getPosition();
	}
	else
	{
		return position;
	}
}