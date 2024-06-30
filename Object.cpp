#include"Object.h"
#include"Camera.h"
#include"Controller.h"

Object::Object()
{
	uniformBufferChange = true;

	position = { 0,0,0 };
	posOffSet = 0.0f;

	forward = glm::vec3{ 0,0,1 };
	right = glm::vec3{ 1,0,0 };

	parentObject = nullptr;
	childObjects.clear();
	spherePos = false;

	controllable = false;
	speed = 10.0f;

	rotateSpeed = 0.1f;
	length = 1.0f;
}

void Object::bindObject(Object* obj)
{
	if (obj == nullptr)
	{
#ifdef _DEBUG
		throw std::runtime_error("bindObject(Object* obj): bindObject is nullptr");
#endif

		return;
	}

	childObjects.push_back(obj);

	obj->setParentObject(this);
}

void Object::bindObject(std::shared_ptr<Camera> camera)
{
	if (bindCamera != nullptr)
	{
#ifdef _DEBUG
		throw std::runtime_error("bindObject(Camera* camera): bindCamera not nullptr");
#endif
		return;
	}

	bindCamera = std::shared_ptr<Camera>(camera);
	camera->setParentObject(this);
}

void Object::setParentObject(Object* obj)
{
	if (obj == nullptr)
	{
#ifdef _DEBUG
		throw std::runtime_error("setParentObject: obj is nullptr");
#endif

		return;
	}

	parentObject = obj;
}

glm::vec3 Object::inputMove()
{
	glm::vec3 moveDirec;
	auto controller = Controller::GetInstance();

	if (bindCamera)
	{
		forward = bindCamera->forward;
		right = bindCamera->right;
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

	for (auto itr = childObjects.begin(); itr != childObjects.end(); itr++)
	{
		//(*itr)->setPosition((*itr)->getPosition() + (pos - position));
	}
	
	if (bindCamera)
	{
		//bindCamera->setPosition(bindCamera->getPosition() + (pos - position));
	}

	position = pos;
}

glm::vec3 Object::getPosition()
{
	return position;
}

void Object::setSpherePos(float theta, float phi)
{
	glm::vec3 pos;
	pos = { posOffSet * cos(theta) * cos(phi),posOffSet * sin(phi),posOffSet * sin(theta) * cos(phi) };
	pos += parentObject->getPosition();

	this->forward = glm::normalize(glm::vec3(pos - this->parentObject->getPosition()));
	this->right = glm::cross(glm::vec3(0, 1, 0), this->forward);

	setPosition(pos);
}