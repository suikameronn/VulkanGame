#include"Camera.h"

Camera::Camera()
{
	position = { 0,0.0f,5.0f };
	distance = 100.0f;
	offsetPos = glm::vec3(0.0f,20.0f,0.0f);

	forward = glm::vec3{ 0,0,1};
	right = glm::vec3{ 1,0,0};
	up = glm::vec3{ 0.0f,1.0f,0.0f };

	theta = 0.0f;
	phi = 0.0f;

	viewAngle = 45.0f;
	aspect = 900.0f / 600.0f;
	viewPointSpeed = 1.0f;

	setPosition(glm::vec3(0, 0, distance));

	zNear = 0.1f;
	zFar = 1000.0f;

	perspectiveMat = glm::perspective(viewAngle, aspect, zNear,zFar);
}

//���W�̐ݒ�
void Camera::setPosition(glm::vec3 pos)
{
	if (position == pos)
	{
		return;
	}

	position = pos;
}

//���ʍ��W��̔��a�̐ݒ�
void Camera::setDistance(float distance)
{
	this->distance = distance;
}

//�Ǐ]����I�u�W�F�N�g�Ƃ̋�����ݒ�
void Camera::setOffsetPos(glm::vec3 offset)
{
	this->offsetPos = offset;
}

//����p�̐ݒ�
void Camera::setViewAngle(float f)
{
	viewAngle = f;

	perspectiveMat = glm::perspective(viewAngle, aspect, zNear, zFar);
}

//����p�̎擾
float Camera::getViewAngle()
{
	return viewAngle;
}

//�J�����̍X�V����
void Camera::Update()
{
	customUpdate();
}

//���L�[����̓��͂�����
void Camera::customUpdate()
{
	auto controller = Controller::GetInstance();

	//���L�[����ړ�����p�x��ݒ� �P�ʂ͓x���@
	if (controller->getKey(GLFW_KEY_LEFT) != GLFW_RELEASE)
	{
		theta -= viewPointSpeed;
	}

	if (controller->getKey(GLFW_KEY_RIGHT) != GLFW_RELEASE)
	{
		theta += viewPointSpeed;
	}

	if (controller->getKey(GLFW_KEY_UP) != GLFW_RELEASE)
	{
		phi += viewPointSpeed;
	}

	if (controller->getKey(GLFW_KEY_DOWN) != GLFW_RELEASE)
	{
		phi -= viewPointSpeed;
	}

	//��]�p�x�̐���
	if (phi >= 90.0f)
	{
		phi = 89.0f;
	}
	else if (phi <= -90.0f)
	{
		phi = -89.0f;
	}

	//���ʍ��W��̍��W�擾
	setSpherePos(glm::radians(theta), glm::radians(-phi));
	//�r���[�s��̌v�Z
	calcViewMat();
}

//�Ǐ]����I�u�W�F�N�g�̈ʒu��ݒ�
void Camera::setParentPos(glm::vec3 position)
{
	parentPos = position;
}

//���ʍ��W�̐ݒ� �����͖��L�[����
void Camera::setSpherePos(float theta, float phi)
{
	glm::vec3 pos;
	pos = { distance * cos(theta) * cos(phi),distance * sin(phi),distance * sin(theta) * cos(phi) };
	pos += parentPos + offsetPos;

	//�J�����̉�]�ɍ��킹�āA�J�����̐��ʂƉE�̃x�N�g�����v�Z
	this->forward = glm::normalize(glm::vec3(pos - parentPos));
	this->right = glm::cross(glm::vec3(0, 1, 0), this->forward);

	setPosition(pos);
}

//�r���[�s��̌v�Z�A�L���[�u�}�b�s���O�p�̍s��������Ɍv�Z
void Camera::calcViewMat()
{
	//�Ǐ]����I�u�W�F�N�g�̍��W�̒��S�������ݒ�
	glm::vec3 pos = glm::vec3(parentPos.x, parentPos.y + 20.0f, parentPos.z);

	viewMat = glm::lookAt(this->position, pos, up);

	//�L���[�u�}�b�v�̗����̂̉�]���J�����̓����ƍ��킹�邽��
	//x,y�����̉�]�̂ݔ��]������
	cubemapViewMat = glm::lookAt(this->position * glm::vec3(-1.0f,1.0f,1.0f), pos * glm::vec3(-1.0f, 1.0f, 1.0f), -up);
}

//�Ǐ]����^�[�Q�b�g�̍��W���擾
glm::vec3 Camera::getViewTarget()
{
	return parentPos;
}

//x�����̈ړ��̎擾
float Camera::getTheta()
{
	return theta;
}

//�t�@�[�N���b�v�͈͂̎擾
void Camera::getzNearFar(float& near, float& far)
{
	near = zNear;
	far = zFar;
}