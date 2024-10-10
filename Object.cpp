#include"Object.h"

Object::Object()
{
	uniformBufferChange = true;

	position = { 0,0,0 };
	posOffSet = 0.0f;

	forward = glm::vec3{ 0,0,1 };
	right = glm::vec3{ 1,0,0 };

	childObjects.clear();
	spherePos = false;

	controllable = false;
	speed = 10.0f;

	rotateSpeed = 0.1f;
	length = 1.0f;
}

void Object::bindObject(std::weak_ptr<Object> obj)
{
	if (obj.expired())
	{
#ifdef _DEBUG
		throw std::runtime_error("bindObject(Object* obj): bindObject is nullptr");
#endif

		return;
	}

	childObjects.push_back(obj);

	sendPosToChildren();
}

void Object::bindCamera(std::weak_ptr<Object> camera)
{
	cameraObj = camera;

	sendPosToChildren();
}

glm::vec3 Object::inputMove()
{
	glm::vec3 moveDirec;
	auto controller = Controller::GetInstance();

	if (cameraObj.expired())
	{
		forward = cameraObj.lock()->forward;
		right = cameraObj.lock()->right;
	}

	if (controller->getKey(GLFW_KEY_W))
	{
		moveDirec = -forward;
	}
	else if (controller->getKey(GLFW_KEY_A))
	{
		moveDirec = -right;
	}
	else if (controller->getKey(GLFW_KEY_D))
	{
		moveDirec = right;
	}
	else if (controller->getKey(GLFW_KEY_S))
	{
		moveDirec = forward;
	}
	else
	{
		moveDirec = { 0,0,0 };
	}

	return moveDirec;
}

void Object::Update()
{
	if (controllable)
	{
		glm::vec3 moveDirec = inputMove();
		setPosition(this->position + moveDirec * speed);
	}
}

void Object::setPosition(glm::vec3 pos)
{

	position = pos;
	sendPosToChildren();
}

glm::vec3 Object::getPosition()
{
	return position;
}

glm::mat4 Object::getTransformMatrix()
{
	return transformMatrix;
}

void Object::setSpherePos(float theta, float phi)
{
	if (theta != 0.0f || phi != 0.0f || true)
	{
		glm::vec3 pos;
		pos = { posOffSet * cos(theta) * cos(phi),posOffSet * sin(phi),posOffSet * sin(theta) * cos(phi) };
		pos += parentPos;

		this->forward = glm::normalize(glm::vec3(pos - parentPos));
		this->right = glm::cross(glm::vec3(0, 1, 0), this->forward);

		setPosition(pos);
	}
}

void Object::sendPosToChildren()
{
	for (auto itr = childObjects.begin(); itr != childObjects.end(); itr++)
	{
		if (!itr->expired())
		{
			itr->lock()->setParentPos(this->position);
		}
	}

	if (!cameraObj.expired())
	{
		cameraObj.lock()->setParentPos(this->position);
	}
}

void Object::setParentPos(glm::vec3 pos)
{
	parentPos = pos;
}