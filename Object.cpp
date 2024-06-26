#include"Object.h"
#include"Controller.h"

Object::Object()
{
	uniformBufferChange = true;

	position = { 0,0,0 };
	posOffSet = { 0,0,0 };

	forward = glm::vec3{ 0,0,1 };
	right = glm::vec3{ 1,0,0 };

	otherObject = nullptr;
	spherePos = false;

	controllable = false;
	speed = 10.0f;

	rotateSpeed = 0.1f;
	length = 1.0f;

	current = { 1.0, 0.0, 0.0, 0.0 };
	after = { 1.0, 0.0, 0.0, 0.0 };
	target = { 1.0,0.0,0.0,0.0 };
	quatMat = glm::mat4(1.0);
}

float Object::convertRadian(float degree)
{
	return degree * (PI / 180);
}

void Object::bindObject(Object* obj)
{
	otherObject = obj;
}

glm::vec3 Object::inputMove()
{
	glm::vec3 moveDirec;
	auto controller = Controller::GetInstance();

	if (controller->getKey(GLFW_KEY_W))
	{
		moveDirec = forward;
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
		moveDirec = -forward;
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
}

glm::vec3 Object::getPosition()
{
	return position;
}

void Object::convertQuatMat()
{
	quatMat = glm::mat4_cast(target);
}

glm::mat4 Object::getQuatMat()
{
	return quatMat;
}

glm::quat Object::makeQuat(glm::vec3 axis, float rad)
{
	glm::quat quaternion;
	float halfSin, halfCos;

	quaternion = { 0,0,0,0 };
	axis = glm::normalize(axis);

	halfSin = sinf(rad * 0.5f);
	halfCos = cosf(rad * 0.5f);

	quaternion.w = halfCos;
	quaternion.x = axis.x * halfSin;
	quaternion.y = axis.y * halfSin;
	quaternion.z = axis.z * halfSin;

	return quaternion;
}

glm::vec3 Object::rotateQuatPosition(glm::vec3 pos, glm::vec3 axis, float rad)
{
	glm::quat complexNumber, complexConjugateNumber;
	glm::quat posQuat = glm::quat{ 0,pos.x,pos.y,pos.z };
	glm::vec3 resultPos;

	if (axis == glm::vec3(0, 0, 0) || rad == 0)
	{
		return pos;
	}

	complexNumber = makeQuat(axis, rad);
	complexConjugateNumber = makeQuat(axis, -rad);

	posQuat = complexNumber * posQuat;
	posQuat = posQuat * complexConjugateNumber;

	resultPos.x = posQuat.x;
	resultPos.y = posQuat.y;
	resultPos.z = posQuat.z;

	return resultPos;
}

void Object::setSpherePos(float theta, float phi)
{
	this->position = rotateQuatPosition(this->position, this->up, theta);

	this->position = rotateQuatPosition(this->position, this->right, phi);

	this->position = glm::normalize(this->position) * 50.0f;

	std::cout << position.x << " " << position.y << " " << position.z << std::endl;
}