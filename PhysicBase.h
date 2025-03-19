#pragma once
#include<time.h>
#include<vector>
#include<memory>
#include<chrono>

#include<iostream>

#include<glm/glm.hpp>

class PhysicBase
{
private:

	//�d��
	glm::vec3 gravity;
	//���x
	glm::vec3 velocity;

public:

	PhysicBase();

	//���x�̍X�V����
	virtual void Update(bool isAccelerateReset = true);
	//���x�̃��Z�b�g
	void setZeroVelocity();
	//�d�͂�ł�����(�n�ʂɑ������Ă���Ƃ�)
	void cancelGravity();
	//���x���������܂߂ĉ�����
	virtual void addVelocity(glm::vec3 addVelocity);
	//���݂̑��x���擾����
	virtual glm::vec3 getVelocity();
};