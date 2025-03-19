#pragma once

#define GLM_ENABLE_EXPERIMENTAL

#include<glm/glm.hpp>
#include<glm/ext.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include"Controller.h"

//���z�I�ȃJ�����̃N���X
class Camera
{
private:
	//���W
	glm::vec3 position;
	//�Ǐ]����I�u�W�F�N�g�̍��W
	glm::vec3 parentPos;
	
	//���ʍ��W��̔��a
	float distance;
	//�Ǐ]����I�u�W�F�N�g�Ƃ̋���
	glm::vec3 offsetPos;

	//��ʂ̔䗦
	float aspect;
	//����p
	float viewAngle;
	//�J�����̈ړ����x
	float viewPointSpeed;

	//���ʍ��W��̊p�x
	float theta, phi;

	//�N���b�v�͈�
	float zNear, zFar;

	//�r���[�s��̌v�Z�A�L���[�u�}�b�s���O�p�̍s��������Ɍv�Z
	void calcViewMat();
public:
	//�J�����̐��ʁA�E�A������̃x�N�g��
	glm::vec3 forward, right, up;

	//�ʏ�̃����_�����O�p�̃r���[�s��
	glm::mat4 viewMat;
	//�L���[�u�}�b�s���O�p�̃r���[�s��A�ʏ�̃r���[�s���y�����̉�]�̂ݔ��]���Ă���
	glm::mat4 cubemapViewMat;
	//�������e�s��
	glm::mat4 perspectiveMat;

	Camera();

	//���W�̐ݒ�
	void setPosition(glm::vec3 pos);
	//���W�̎擾
	glm::vec3 getPosition() { return position; }
	//�Ǐ]����I�u�W�F�N�g�̈ʒu��ݒ�
	void setParentPos(glm::vec3 position);
	//���ʍ��W��̔��a�̐ݒ�
	void setDistance(float distance);
	//�Ǐ]����I�u�W�F�N�g�Ƃ̋�����ݒ�
	void setOffsetPos(glm::vec3 offset);

	//���ʍ��W�̐ݒ� �����͖��L�[����
	void setSpherePos(float theta, float phi);

	//����p�̐ݒ�
	void setViewAngle(float f);
	//����p�̎擾
	float getViewAngle();
	//x�����̈ړ��̎擾
	float getTheta();

	//�t�@�[�N���b�v�͈͂̎擾
	void getzNearFar(float& near, float& far);

	//�Ǐ]����^�[�Q�b�g�̍��W���擾
	glm::vec3 getViewTarget();

	//�J�����̍X�V����
	void Update();
	//���L�[����̓��͂�����
	void customUpdate();
};