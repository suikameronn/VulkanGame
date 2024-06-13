#include"Object.h"

Object::Object()
{
	uniformBufferChange = true;

	position = { 0,0,0 };
	posOffSet = { 0,0,0 };

	forward = glm::vec3{ 0,0,1 };
	right = glm::vec3{ 1,0,0 };

	otherObject = nullptr;
	spherePos = false;

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

void Object::Update()
{

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



void Object::setSpherePos(glm::vec3 center, float r, float theta, float phi)
{
	//phi = convertRadian(phi);
	//theta = convertRadian(theta);

	//glm::vec3 pos = { r * sin(phi) * cos(theta) ,r * cos(phi),-r * sin(phi) * sin(theta) };

	

	float tmp = theta - theta2;
	float tmp2 = phi - phi2;

	std::cout << tmp << " " << tmp2 << std::endl;
	std::cout << " " << std::endl;

	if (tmp > 0.0 || tmp2 > 0.0)
	{
		float length = sqrt(tmp * tmp + tmp2 * tmp2);
		float as = sin(length) / length;
		glm::quat after = { cos(length), tmp2 * as, tmp * as, 0.0 };

		target = glm::cross(after, current);


		convertQuatMat();
	}
	else
	{
		current = target;
	}
}